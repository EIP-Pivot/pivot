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
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTextCoords;
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

vec3 calculateSpotLight(in Material mat, in SpotLight light)
{
    vec3 lightDir = normalize(light.position.xyz - fragPosition);
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0) * light.intensity;

    vec3 norm = normalize(fragNormal);

    // diffuse
    float diffuse = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 refectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(norm, refectDir), 0.0), 32.0);

    return light.color.rgb * ((diffuse + specular) * intensity);
}

vec3 calculateDirectionalLight(in Material mat, in DirectionalLight light)
{
    vec3 lightDir = normalize(-light.orientation.xyz);
    vec3 norm = normalize(fragNormal);

    // diffuse
    float diffuse = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 refectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(norm, refectDir), 0.0), 32.0);

    return light.color.rgb * ((diffuse + specular) * light.intensity);
}

vec3 calculateLight(in Material mat, in PointLight light)
{
    vec3 lightDir = normalize(light.position.xyz - fragPosition);
    vec3 norm = normalize(fragNormal);

    // diffuse
    float diffuse = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 refectDir = reflect(-lightDir, norm);
    float specular = pow(max(dot(norm, refectDir), 0.0), 32.0);

    float attenuation = 1.0 / length(light.position.xyz - fragPosition) * light.falloff;
    diffuse *= attenuation;
    specular *= attenuation;

    return light.color.rgb * ((diffuse + specular) * light.intensity);
}

void main()
{
    Material material = objectMaterials.materials[materialIndex];
    vec3 diffuseColor = (material.baseColorTexture >= 0)
                            ? (texture(texSampler[material.baseColorTexture], fragTextCoords).rgb)
                            : (material.baseColor.rgb);

    vec3 light = vec3(0.1);
    for (uint i = 0; i < cameraData.push.directLightCount; i++) {
        light += calculateDirectionalLight(material, directLight.directionalLightArray[i]);
    }
    for (uint i = 0; i < cameraData.push.spotLightCount; i++) {
        light += calculateSpotLight(material, spotLight.spotLightArray[i]);
    }
    for (uint i = 0; i < cameraData.push.pointLightCount; i++) {
        light += calculateLight(material, omniLight.pointLightArray[i]);
    }
    outColor = vec4(light * diffuseColor, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));
}
