#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;

void main()
{
    vec4 worldPos = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    vec3 worldNormal = normalize(mat3(ubo.model) * inNormal);

    vPosition = worldPos.xyz;
    vNormal = worldNormal;
    vTexCoord = inTexCoord;

    gl_Position = worldPos;
}
