#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextCoords;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTextCoords;
layout(location = 3) out vec3 fragColor;
layout(location = 4) out vec4 fragTangent;
layout(location = 5) out uint materialIndex;

struct UniformBufferObject {
    mat4 modelMatrix;
    uint materialIndex;
    uint boundingBoxIndex;
};

layout(std140, set = 2, binding = 1) readonly buffer ObjectBuffer
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
    mat4 matrix = objectBuffer.objects[gl_BaseInstance].modelMatrix;
    vec3 locPos = vec3(matrix * vec4(inPosition, 1.0));

    fragPosition = locPos;
    fragNormal = mat3(matrix) * inNormal;
    fragTextCoords = inTextCoords;
    fragColor = inColor;
    fragTangent = vec4(mat3(matrix) * inTangent.xyz, inTangent.w);

    materialIndex = objectBuffer.objects[gl_BaseInstance].materialIndex;

    gl_Position = cameraData.viewproj * vec4(fragPosition, 1.0);
}
