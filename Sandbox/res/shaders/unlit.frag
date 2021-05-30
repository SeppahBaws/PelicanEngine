#version 450

layout(location = 0) in vec3 vColor;
layout(location = 0) out vec3 fragColor;

void main()
{
    fragColor = vec4(fragColor, 1.0);
}
