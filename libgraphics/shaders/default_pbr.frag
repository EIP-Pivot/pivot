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

layout(std140, set = 0, binding = 1) readonly buffer ObjectMaterials
{
    Material materials[];
}
objectMaterials;

layout(set = 0, binding = 2) uniform sampler2D texSampler[NUMBER_OF_TEXTURES];

layout(std140, set = 1, binding = 2) readonly buffer LightBuffer
{
    PointLight pointLightArray[];
}
omniLight;

layout(std140, set = 1, binding = 3) readonly buffer DirectLight
{
    DirectionalLight directionalLightArray[];
}
directLight;

layout(std140, set = 1, binding = 4) readonly buffer SpoLight
{
    SpotLight spotLightArray[];
}
spotLight;

#define PI 3.1415926535897932384626433832795

#define SAMPLE_OPTIONAL_WITH_DEFAULT(name, swizzle, defaultValue)                                         \
    (material.name##Texture >= 0) ? (texture(texSampler[material.name##Texture], fragTextCoords).swizzle) \
                                  : (defaultValue)

#define SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(name, defaultValue) SAMPLE_OPTIONAL_WITH_DEFAULT(name, rgb, defaultValue)

vec3 getNormalFromMap(const in Material material)
{
    /// The tangent have an issue when loaded. Can't use atm.
    // vec3 tangentNormal = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(normal, vec3(1.0));
    // tangentNormal *= 2.0 - 1.0;

    // vec3 N = normalize(fragNormal);
    // vec3 T = normalize(fragTangent.xyz);
    // vec3 B = normalize(cross(N, T));
    // mat3 TBN = mat3(T, B, N);
    // return normalize(TBN * tangentNormal);
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

float DistributionGGX(float dotNH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nom = a2;
    float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;

    return nom / max(PI * denom * denom, 0.001);
}

float GeometrySchlickGGX(float dotNL, float dotNV, float roughness)
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

vec3 fresnelSchlickR(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 diffuseColor, float metallic, float roughness)
{
    vec3 H = normalize(V + L);
    float dotNH = max(dot(N, H), 0.001);
    float dotNV = max(dot(N, V), 0.1);
    float dotNL = max(dot(N, L), 0.0);

    vec3 color = vec3(0.0);
    if (dotNL > 0.0) {
        float D = DistributionGGX(dotNH, roughness);
        float G = GeometrySchlickGGX(dotNL, dotNV, roughness);
        vec3 F = fresnelSchlick(dotNV, F0);

        vec3 spec = D * F * G / max(4.0 * dotNL * dotNV, 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

        color += (kD * diffuseColor / PI + spec) * dotNL;
    }
    return color;
}

void main()
{
    Material material = objectMaterials.materials[materialIndex];
    vec4 diffuseColor_ = SAMPLE_OPTIONAL_WITH_DEFAULT(baseColor, rgba, material.baseColor);
    vec3 metallicRoughness = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(metallicRoughness, vec3(1.0));
    float occlusion = SAMPLE_OPTIONAL_WITH_DEFAULT(occlusion, r, 1.0);

    float metallic = metallicRoughness.b * material.metallic;
    float roughness = metallicRoughness.g * material.roughness;
    vec4 diffuseColor = diffuseColor_ * material.baseColorFactor * vec4(fragColor, 1.0);

    if (diffuseColor.a < material.alphaCutOff)
        discard;
    vec3 N = getNormalFromMap(material);
    vec3 V = normalize(cameraData.push.position - fragPosition);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, diffuseColor.rgb, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < cameraData.push.directLightCount; i++) {
        DirectionalLight light = directLight.directionalLightArray[i];
        vec3 radiance = light.color.rgb * light.intensity;

        vec3 L = normalize(-light.orientation.xyz);
        Lo += specularContribution(L, V, N, F0, diffuseColor.rgb, metallic, roughness) * radiance;
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

        Lo += specularContribution(L, V, N, F0, diffuseColor.rgb, metallic, roughness) * radiance;
    }
    for (uint i = 0; i < cameraData.push.pointLightCount; i++) {
        PointLight light = omniLight.pointLightArray[i];
        vec3 L = normalize(light.position.xyz - fragPosition);

        float distance = length(light.position.xyz - fragPosition);
        float attenuation = 1.0 / ((distance * distance) * light.falloff);
        vec3 radiance = light.color.rgb * light.intensity * attenuation;

        Lo += specularContribution(L, V, N, F0, diffuseColor.rgb, metallic, roughness) * radiance;
    }

    vec3 ambient = vec3(0.03) * diffuseColor.rgb * occlusion;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, diffuseColor.a);
}
