#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
    float minRadius;
    float radius;
    float falloff;
};

struct DirectionalLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
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
    vec3 position;
};

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTextCoords;
layout(location = 4) in flat uint materialIndex;

layout(location = 0) out vec4 outColor;


layout (push_constant) uniform readonly constants {
    layout(offset = 64) pushConstantStruct push;
} cameraData;

layout (std140, set = 0, binding = 1) readonly buffer ObjectMaterials {
    Material materials[];
} objectMaterials;

layout(set = 0, binding = 2) uniform sampler2D texSampler[];

layout(std140, set = 1, binding = 2) readonly buffer LightBuffer {
    PointLight pointLightArray[];
}  omniLight;

layout(std140, set = 1, binding = 3) readonly buffer DirectLight {
    DirectionalLight pointLightArray[];
}  directLight;

const float ambientStrength = 0.1;
const float specularStrength = 0.5;
const float diffuseStrength = 5.0;

vec3 calculateLight(in  Material mat, in PointLight light) {
    // ambient
    vec3 ambient = ambientStrength * vec3(1.0);

    // diffuse
    vec3 lightDir = normalize(light.position.xyz - fragPosition);
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * (diff * vec3(1.0));

    // specular
    vec3 viewDir = normalize(cameraData.push.position - fragPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = specularStrength *  (spec * vec3(1.0));

    float attenuation = 1.0 / length(light.position.xyz - fragPosition);
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main() {
    Material material = objectMaterials.materials[materialIndex];
    vec3 diffuseColor = (material.baseColorTexture >= 0) ? (texture(texSampler[material.baseColorTexture], fragTextCoords).rgb) : (material.baseColor.rgb);
    
    vec3 light = vec3(1.0);
    for (uint i = 0; i < cameraData.push.pointLightCount; i++) {
        light += calculateLight(material, omniLight.pointLightArray[i]);
    }
    outColor =  vec4(light * diffuseColor, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0/gamma));
}
