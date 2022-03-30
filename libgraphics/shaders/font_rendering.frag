#version 460
#extension GL_EXT_nonuniform_qualifier : enable


layout (location = 0) in vec2 inTextCoords;

layout (location = 0) out vec4 outColor;

struct Character {
    int textureId;
    ivec2 bearing;
    uint advance;
};

layout(set = 0, binding = 2) uniform sampler2D texSampler[];
layout (set = 0, binding = 3 ) uniform readonly constants {
    Character character[];
} charData;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(texSampler[charData.character[0].textureId], inTextCoords).r);
    outColor = vec4(vec3(1.0, 1.0, 1.0), 1.0) * sampled;
}  
