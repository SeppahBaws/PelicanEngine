#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;

layout(location = 0) out vec3 vPosition;

void main()
{
    vPosition = inPosition;

    // Convert cubemap coordinates into Vulkan coordinate space
    vPosition.x *= -1.0;
    gl_Position = ubo.proj * ubo.model * vec4(inPosition.xyz, 1.0);
    gl_Position.y *= -1.0;
}
