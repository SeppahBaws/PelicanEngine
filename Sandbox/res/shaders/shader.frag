#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texNormal;
layout(binding = 3) uniform sampler2D texMetallicRoughness;

const vec3 lightDir = vec3(0.577, -0.577, -0.577);

void main()
{
    vec3 finalColor;
    
    vec3 baseColor = texture(texAlbedo, vTexCoord).rgb;
    vec3 sampledNormal = texture(texNormal, vTexCoord).rgb;
    vec3 metallicRoughness = texture(texMetallicRoughness, vTexCoord).rgb;

    vec3 normal = normalize(vNormal * sampledNormal);
    
    float diffuseStrength = dot(normal, -lightDir);
    diffuseStrength = diffuseStrength * 0.5 + 0.5;
    diffuseStrength = clamp(diffuseStrength, 0, 1);
    finalColor = baseColor * diffuseStrength;

    float alpha = texture(texAlbedo, vTexCoord).a;
    if (alpha <= 0.001f)
        discard; // The discard helps a bit with the current transparency issue, although it should still be fixed properly later on.

    outColor = vec4(finalColor, alpha);
}
