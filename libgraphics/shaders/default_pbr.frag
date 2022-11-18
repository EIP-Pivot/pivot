#version 460

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
    float falloff;
};

struct DirectionalLight {
    vec4 orientation;
    vec4 color;
    float intensity;
};

struct SpotLight {
    vec4 position;
    vec4 direction;
    vec4 color;
    float cutOff;
    float outerCutOff;
    float intensity;
};

struct Material {
    float alphaCutOff;
    float metallic;
    float roughness;
    vec4 baseColor;
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    int baseColorTexture;
    int metallicRoughnessTexture;
    int normalTexture;
    int occlusionTexture;
    int emissiveTexture;
    int specularGlossinessTexture;
    int diffuseTexture;
};

struct pushConstantStruct {
    uint pointLightCount;
    uint directLightCount;
    uint spotLightCount;
    vec3 position;
};

layout(constant_id = 0) const uint NUMBER_OF_TEXTURES = 1;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTextCoords;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in vec4 fragTangent;
layout(location = 5) in flat uint materialIndex;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform readonly constants
{
    layout(offset = 64) pushConstantStruct push;
}
cameraData;

layout(std140, set = 2, binding = 1) readonly buffer ObjectMaterials
{
    Material materials[];
}
objectMaterials;

layout(set = 2, binding = 2) uniform sampler2D texSampler[NUMBER_OF_TEXTURES];

layout(std140, set = 1, binding = 0) readonly buffer DirectLight
{
    DirectionalLight directionalLightArray[];
}
directLight;

layout(std140, set = 1, binding = 1) readonly buffer SpoLight
{
    SpotLight spotLightArray[];
}
spotLight;

layout(std140, set = 1, binding = 2) readonly buffer LightBuffer
{
    PointLight pointLightArray[];
}
omniLight;

#define PI 3.1415926535897932384626433832795

#define SAMPLE_OPTIONAL_WITH_DEFAULT(name, swizzle, defaultValue)                                         \
    (material.name##Texture >= 0) ? (texture(texSampler[material.name##Texture], fragTextCoords).swizzle) \
                                  : (defaultValue)

#define SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(name, defaultValue) SAMPLE_OPTIONAL_WITH_DEFAULT(name, rgb, defaultValue)

struct PBRInfo {
    vec3 N;                       // Normal vector
    vec3 V;                       // Vector from surface point to camera
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;            // color contribution from diffuse lighting
    vec3 specularColor;           // color contribution from specular lighting
};

vec3 getNormalFromMap(const in Material material)
{
    vec3 tangentNormal = (material.normalTexture >= 0)
                             ? (texture(texSampler[material.normalTexture], fragTextCoords).xyz)
                             : (vec3(1.0)) * 2.0 - 1.0;

    vec3 Q1 = dFdx(fragPosition);
    vec3 Q2 = dFdy(fragPosition);
    vec2 st1 = dFdx(fragTextCoords);
    vec2 st2 = dFdy(fragTextCoords);

    vec3 N = normalize(fragNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float microfacetDistribution(float dotNH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nom = a2;
    float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;

    return nom / max(PI * denom * denom, 0.001);
}

float geometricOcclusion(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / max(dotNL * (1.0 - k) + k, 0.001);
    float GV = dotNV / max(dotNV * (1.0 - k) + k, 0.001);
    return GL * GV;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 specularReflection(float dotVH, vec3 reflectance0, vec3 reflectance90)
{
    return reflectance0 + (reflectance90 - reflectance0) * pow(clamp(1.0 - dotVH, 0.0, 1.0), 5.0);
}

vec3 specularContribution(vec3 L, PBRInfo info)
{
    // black metallic fix (temp) proper fix need implementation of IRB
    if (info.reflectance0 == vec3(0)) {
        info.reflectance0 = vec3(10);
    }

    vec3 H = normalize(info.V + L);
    float dotNH = clamp(dot(info.N, H), 0.0, 1.0);                // cos angle between normal and half vector
    float dotNV = clamp(abs(dot(info.N, info.V)), 0.001, 1.0);    // cos angle between normal and view direction
    float dotNL = clamp(dot(info.N, L), 0.001, 1.0);              // cos angle between normal and light direction
    float dotVH = clamp(dot(info.V, H), 0.0, 1.0);                // cos angle between view direction and half vector
    float dotLH = clamp(dot(L, H), 0.0, 1.0);                     // cos angle between light direction and half vector

    vec3 color = vec3(0.0);
    if (dotNL > 0.0) {
        // Calculate the shading terms for the microfacet specular shading model
        vec3 F = specularReflection(dotVH, info.reflectance0, info.reflectance90);
        float G = geometricOcclusion(dotNL, dotNV, info.perceptualRoughness);
        float D = microfacetDistribution(dotNH, info.perceptualRoughness);

        vec3 spec = D * F * G / max(4.0 * dotNL * dotNV, 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - info.metalness);

        color += (kD * info.diffuseColor / PI + spec) * dotNL;
    }
    return color;
}

void main()
{
    Material material = objectMaterials.materials[materialIndex];
    vec4 baseColor_ = SAMPLE_OPTIONAL_WITH_DEFAULT(baseColor, rgba, material.baseColor);
    vec4 baseColor = baseColor_ * material.baseColorFactor * vec4(fragColor, 1.0);
    if (baseColor.a < material.alphaCutOff)
        discard;

    vec3 metallicRoughness = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(metallicRoughness, vec3(1.0));
    float occlusion = SAMPLE_OPTIONAL_WITH_DEFAULT(occlusion, r, 1.0);

    float metallic = metallicRoughness.b * material.metallic;
    float roughness = metallicRoughness.g * material.roughness;

    vec3 N = getNormalFromMap(material);
    vec3 V = normalize(cameraData.push.position - fragPosition);

    vec3 F0 = vec3(0.04);
    vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - F0);
    diffuseColor *= 1.0 - metallic;

    vec3 specularColor = mix(F0, diffuseColor.rgb, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);

    PBRInfo pbrInfo = PBRInfo(N, V, roughness, metallic, vec3(specularColor), vec3(reflectance90),
                              roughness * roughness, diffuseColor.rgb, F0);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < cameraData.push.directLightCount; i++) {
        DirectionalLight light = directLight.directionalLightArray[i];
        vec3 radiance = light.color.rgb * light.intensity;

        vec3 L = normalize(-light.orientation.xyz);
        Lo += specularContribution(L, pbrInfo) * radiance;
    }
    for (uint i = 0; i < cameraData.push.spotLightCount; i++) {
        SpotLight light = spotLight.spotLightArray[i];
        vec3 L = normalize(light.position.xyz - fragPosition);

        float theta = dot(L, normalize(-light.direction.xyz));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = (theta - light.outerCutOff) / epsilon * light.intensity;

        float distance = length(light.position.xyz - fragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light.color.rgb * intensity * attenuation;

        Lo += specularContribution(L, pbrInfo) * radiance;
    }
    for (uint i = 0; i < cameraData.push.pointLightCount; i++) {
        PointLight light = omniLight.pointLightArray[i];
        vec3 L = normalize(light.position.xyz - fragPosition);

        float distance = length(light.position.xyz - fragPosition);
        float attenuation = 1.0 / ((distance * distance) * light.falloff);
        vec3 radiance = light.color.rgb * light.intensity * attenuation;

        Lo += specularContribution(L, pbrInfo) * radiance;
    }

    vec3 emissive = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(emissive, vec3(1.0)) * material.emissiveFactor.rgb;

    vec3 ambient = vec3(0.03) * diffuseColor.rgb * occlusion;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, baseColor.a);
}
