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

layout (std140, set = 0, binding = 1) readonly buffer ObjectMaterials {
    Material materials[];
} objectMaterials;

layout(set = 0, binding = 2) uniform sampler2D texSampler[];


void main() {
    Material material = objectMaterials.materials[materialIndex];
    vec3 diffuseColor = (material.baseColorTexture >= 0) ? (texture(texSampler[material.baseColorTexture], fragTextCoords).rgb) : (material.baseColor.rgb);
    outColor =  vec4(diffuseColor, 1.0);
}
