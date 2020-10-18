#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    outColor = texture(texSampler, vTexCoord);
    outColor = vec4(abs(vPos), 1.0);
    // outColor = vec4(vColor, 1.0);
}
