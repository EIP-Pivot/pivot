#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTextCoords;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosition;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTextCoords;
layout(location = 4) out uint textureIndex;
layout(location = 5) out uint materialIndex;

struct Transform {
    mat4 translation;
    mat4 rotation;
    mat4 scale;
};

struct UniformBufferObject {
    Transform transform;
    uint textureIndex;
    uint materialIndex;
};

layout (std140, set = 0, binding = 0) readonly buffer ObjectBuffer {
    UniformBufferObject objects[];
} objectBuffer;

layout (push_constant) uniform constants {
    vec4 position;
	mat4 viewproj;
} cameraData;


void main() {
    Transform ubo = objectBuffer.objects[gl_BaseInstance].transform;
    mat4 modelMatrix = ubo.translation * ubo.rotation  * ubo.scale;

    gl_Position = cameraData.viewproj * modelMatrix * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragPosition = vec3(modelMatrix * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(modelMatrix))) * inNormal;
    fragTextCoords = inTextCoords;
    textureIndex = objectBuffer.objects[gl_BaseInstance].textureIndex;
    materialIndex = objectBuffer.objects[gl_BaseInstance].materialIndex;
}
