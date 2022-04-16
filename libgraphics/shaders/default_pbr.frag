#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTextCoords;
layout(location = 4) in flat uint materialIndex;

layout(location = 0) out vec4 outColor;

layout (push_constant) uniform readonly constants {
    layout(offset = 64) vec3 position;
} cameraData;

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

layout (std140, set = 1, binding = 1) readonly buffer ObjectMaterials {
    Material materials[];
} objectMaterials;

layout(set = 1, binding = 2) uniform sampler2D texSampler[];

const vec3 lightDirection = vec3(0.0, 1000.0, 0.0);

#define PI 3.1415926535897932384626433832795

vec3 getNormalFromMap(const in Material mat)
{
    vec3 tangentNormal = (mat.normalTexture >= 0) ? (texture(texSampler[mat.normalTexture], fragTextCoords).xyz) : (vec3(1.0)) * 2.0 - 1.0;

    vec3 Q1  = dFdx(fragPosition);
    vec3 Q2  = dFdy(fragPosition);
    vec2 st1 = dFdx(fragTextCoords);
    vec2 st2 = dFdy(fragTextCoords);

    vec3 N   = normalize(fragNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    Material material = objectMaterials.materials[materialIndex];
    vec3 diffuseColor = (material.baseColorTexture >= 0) ? (texture(texSampler[material.baseColorTexture], fragTextCoords).rgb) : (material.baseColor.rgb);
    vec3 metallicRoughness = (material.metallicRoughnessTexture >= 0) ? (texture(texSampler[material.metallicRoughnessTexture], fragTextCoords).rgb) : (vec3(1.0));
    float metallic  = metallicRoughness.b * material.metallic;
    float roughness = metallicRoughness.g * material.roughness;
    float occlusion = (material.occlusionTexture >= 0) ? (texture(texSampler[material.occlusionTexture], fragTextCoords).r) : (1.0);

    vec3 N = getNormalFromMap(material);
    vec3 V = normalize(cameraData.position - fragPosition);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, diffuseColor, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    // Loop this for multiple light
    {
        vec3 L = normalize(lightDirection - fragPosition);
        vec3 H = normalize(V + L);
        float distance = length(lightDirection - fragPosition);
        float attenuation = 1.0 / (distance * distance);
        /// Replace the value with the color of the light
        vec3 radiance = vec3(1000000.0) * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * diffuseColor / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * diffuseColor * occlusion;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}
