#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosition;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTextCoords;
layout(location = 4) flat in uint textureIndex;
layout(location = 5) flat in uint materialIndex;


layout(location = 0) out vec4 outColor;

layout (push_constant) uniform constants {
    vec4 position;
	mat4 viewproj;
} cameraData;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

struct Material {
    vec4 ambientColor;
    vec4 diffuse;
    vec4 specular;
};

layout (std140, set = 0, binding = 1) readonly buffer ObjectMaterials {
    Material materials[];
} objectMaterials;

const vec3 lightDirection = vec3(11.0, 16.0, 24.0);

vec4 calculateLight(Material mat) {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * mat.ambientColor.xyz;

    // diffuse
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightDirection - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * mat.diffuse.xyz;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraData.position.xyz - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * mat.specular.xyz;

    return vec4(ambient + diffuse + specular, 1.0);
}

void main() {
    vec4 light = calculateLight(objectMaterials.materials[materialIndex]);
    outColor = light * texture(texSampler[textureIndex], fragTextCoords);
}
