#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

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
    vec4 baseColor;
    float metallic;
    float roughness;
    int baseColorTexture;
    int metallicRoughnessTexture;
    int normalTexture;
    int occlusionTexture;
    int emissiveTexture;
};

struct pushConstantStruct {
    uint pointLightCount;
    uint directLightCount;
    uint spotLightCount;
    vec3 position;
};

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTextCoords;
layout(location = 3) in vec4 fragTangent;
layout(location = 4) in flat uint materialIndex;

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

layout(set = 0, binding = 2) uniform sampler2D texSampler[];

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
    vec3 tangentNormal = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(normal, vec3(1.0));
    tangentNormal *= 2.0 - 1.0;

    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent.xyz);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

float DistributionGGX(float dotNH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nom = a2;
    float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;

    return nom / max(PI * denom * denom, 0.0001);
}

float GeometrySchlickGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / max(dotNL * (1.0 - k) + k, 0.000001);
    float GV = dotNV / max(dotNV * (1.0 - k) + k, 0.000001);
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
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

    vec3 color = vec3(0.0);
    if (dotNL > 0.0) {
        float D = DistributionGGX(dotNH, roughness);
        float G = GeometrySchlickGGX(dotNL, dotNV, roughness);
        vec3 F = fresnelSchlick(dotNV, F0);
        vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.0001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

        color += (kD * diffuseColor / PI + spec) * dotNL;
    }
    return color;
}

void main()
{
    Material material = objectMaterials.materials[materialIndex];
    vec3 diffuseColor = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(baseColor, material.baseColor.rgb);
    vec3 metallicRoughness = SAMPLE_OPTIONAL_WITH_DEFAULT_RGB(metallicRoughness, vec3(1.0));
    float occlusion = SAMPLE_OPTIONAL_WITH_DEFAULT(occlusion, r, 1.0);

    float metallic = metallicRoughness.b * material.metallic;
    float roughness = metallicRoughness.g * material.roughness;

    vec3 N = getNormalFromMap(material);
    vec3 V = normalize(cameraData.push.position - fragPosition);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, diffuseColor, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < cameraData.push.directLightCount; i++) {
        DirectionalLight light = directLight.directionalLightArray[i];
        vec3 radiance = light.color.rgb * light.intensity;

        vec3 L = normalize(-light.orientation.xyz);
        Lo += specularContribution(L, V, N, F0, diffuseColor, metallic, roughness) * radiance;
    }
    for (uint i = 0; i < cameraData.push.spotLightCount; i++) {
        SpotLight light = spotLight.spotLightArray[i];
        vec3 L = normalize(light.position.xyz - fragPosition);

        float theta = dot(L, normalize(-light.direction.xyz));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0) * light.intensity;

        float distance = length(light.position.xyz - fragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light.color.rgb * intensity * attenuation;

        Lo += specularContribution(L, V, N, F0, diffuseColor, metallic, roughness) * radiance;
    }
    for (uint i = 0; i < cameraData.push.pointLightCount; i++) {
        PointLight light = omniLight.pointLightArray[i];
        vec3 L = normalize(light.position.xyz - fragPosition);

        float distance = length(light.position.xyz - fragPosition);
        float attenuation = 1.0 / ((distance * distance) * light.falloff);
        vec3 radiance = light.color.rgb * light.intensity * attenuation;

        Lo += specularContribution(L, V, N, F0, diffuseColor, metallic, roughness) * radiance;
    }

    vec3 ambient = vec3(0.03) * diffuseColor * occlusion;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
