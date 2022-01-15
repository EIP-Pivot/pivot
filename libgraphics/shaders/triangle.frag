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

layout (push_constant) uniform readonly constants {
    layout(offset = 64) vec3 position;
} cameraData;

struct Material {
    float shininess;
    vec3 ambientColor;
    vec3 diffuse;
    vec3 specular;
};

layout (std140, set = 1, binding = 1) readonly buffer ObjectMaterials {
    Material materials[];
} objectMaterials;

layout(set = 1, binding = 2) uniform sampler2D texSampler[];

const vec3 lightDirection = vec3(11.0, 16.0, 24.0);

const float ambientStrength = 0.1;
const float specularStrength = 0.5;
const float diffuseStrength = 5.0;

vec3 calculateLight(in Material mat) {
    // ambient
    vec3 ambient = ambientStrength * mat.ambientColor;

    // diffuse
    vec3 lightDir = normalize(lightDirection - fragPosition);
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * (diff * mat.diffuse);

    // specular
    vec3 viewDir = normalize(cameraData.position - fragPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = specularStrength *  (spec * mat.specular);

    float attenuation = 1.0 / length(lightDirection - fragPosition);
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main() {
    vec3 diffuseColor = texture(texSampler[textureIndex], fragTextCoords).rgb;
    vec3 light = calculateLight(objectMaterials.materials[materialIndex]);
    outColor =  vec4(light * diffuseColor, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0/gamma));
}
