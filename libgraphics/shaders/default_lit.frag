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

vec3 calculateLight(vec3 lightDir, vec3 lightColor, float intensity)
{
    vec3 norm = normalize(fragNormal);

    // diffuse
    float diffuse = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 refectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(norm, refectDir), 0.0), 32.0);

    return lightColor * ((diffuse + specular) * intensity);
}

void main()
{
    Material material = objectMaterials.materials[materialIndex];

    vec4 diffuseColor = (material.baseColorTexture >= 0)
                            ? (texture(texSampler[material.baseColorTexture], fragTextCoords))
                            : (material.baseColor);
    diffuseColor *= vec4(fragColor, 1.0);
    if (diffuseColor.a < material.alphaCutOff)
        discard;

    vec3 light = vec3(0.1);
    for (uint i = 0; i < cameraData.push.directLightCount; i++) {
        DirectionalLight directionalLight = directLight.directionalLightArray[i];
        vec3 lightDir = normalize(-directionalLight.orientation.xyz);
        light += calculateLight(lightDir, directionalLight.color.rgb, directionalLight.intensity);
    }
    for (uint i = 0; i < cameraData.push.spotLightCount; i++) {
        SpotLight spotLight = spotLight.spotLightArray[i];
        vec3 lightDir = normalize(spotLight.position.xyz - fragPosition);

        float theta = dot(lightDir, normalize(-spotLight.direction.xyz));
        float epsilon = spotLight.cutOff - spotLight.outerCutOff;
        float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0) * spotLight.intensity;

        light += calculateLight(lightDir, spotLight.color.rgb, spotLight.intensity);
    }
    for (uint i = 0; i < cameraData.push.pointLightCount; i++) {
        PointLight pointLight = omniLight.pointLightArray[i];
        vec3 lightDir = normalize(pointLight.position.xyz - fragPosition);
        float attenuation = 1.0 / length(pointLight.position.xyz - fragPosition) * pointLight.falloff;

        light += calculateLight(lightDir, pointLight.color.rgb, pointLight.intensity * attenuation);
    }
    outColor = vec4(light, 1.0) * diffuseColor;

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));
}
