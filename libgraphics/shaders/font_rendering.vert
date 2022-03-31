#version 460

layout (location = 0) in vec4 vertex;
layout (location = 0) out vec2 outTextCoords;

layout (push_constant) uniform readonly constants {
	mat4 projection;
} cameraData;

void main()
{
    gl_Position = cameraData.projection * vec4(vertex.xy, 0.0, 1.0);
    outTextCoords = vertex.zw;
}  
