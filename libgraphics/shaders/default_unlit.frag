#version 460

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
    vec3 position;
};

layout(constant_id = 0) const uint NUMBER_OF_TEXTURES = 1;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTextCoords;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in flat uint materialIndex;

layout(location = 0) out vec4 outColor;

layout(std140, set = 2, binding = 1) readonly buffer ObjectMaterials
{
    Material materials[];
}
objectMaterials;

layout(set = 2, binding = 2) uniform sampler2D texSampler[NUMBER_OF_TEXTURES];

layout(push_constant) uniform readonly constants
{
    layout(offset = 64) pushConstantStruct push;
}
cameraData;

void main()
{
    Material material = objectMaterials.materials[materialIndex];
    vec4 diffuseColor = (material.baseColorTexture >= 0)
                            ? (texture(texSampler[material.baseColorTexture], fragTextCoords))
                            : (material.baseColor);

    diffuseColor *= vec4(fragColor, 1.0) * material.baseColorFactor;
    if (diffuseColor.a < material.alphaCutOff)
        discard;
    outColor = diffuseColor;
}
