#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform samplerCube cubemap;

void main()
{
    fragColor = texture(cubemap, vPosition);
}
