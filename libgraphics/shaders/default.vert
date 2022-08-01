#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextCoords;
layout(location = 3) in vec3 inColor;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTextCoords;
layout(location = 3) out vec3 fragColor;
layout(location = 4) out uint materialIndex;

struct UniformBufferObject {
    mat4 modelMatrix;
    uint materialIndex;
    int boundingBoxIndex;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
    UniformBufferObject objects[];
}
objectBuffer;

layout(push_constant) uniform readonly constants
{
    mat4 viewproj;
}
cameraData;

void main()
{
    gl_Position = cameraData.viewproj * objectBuffer.objects[gl_BaseInstance].modelMatrix * vec4(inPosition, 1.0);

    fragPosition = inPosition;
    fragNormal = inNormal;
    fragTextCoords = inTextCoords;
    fragColor = inColor;
    materialIndex = objectBuffer.objects[gl_BaseInstance].materialIndex;
}
