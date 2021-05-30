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
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec3 vTangent;

void main()
{
    vPosition = (ubo.model * vec4(inPosition, 1.0)).xyz;
    vNormal = normalize(mat3(ubo.model) * inNormal);
    vTexCoord = inTexCoord;
    vTangent = normalize(mat3(ubo.model) * inTangent);

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}
