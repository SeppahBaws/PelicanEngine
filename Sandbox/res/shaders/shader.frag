#version 450

struct DirectionalLight
{
    vec3 direction;
    vec3 lightColor;
    vec3 ambientColor;
};

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 0, binding = 1) uniform LightInformation
{
    DirectionalLight directionalLight;
} lights;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D texAlbedo;
layout(binding = 3) uniform sampler2D texNormal;
layout(binding = 4) uniform sampler2D texMetallicRoughness;

void main()
{
    vec3 finalColor;
    
    vec3 baseColor = texture(texAlbedo, vTexCoord).rgb;
    vec3 sampledNormal = texture(texNormal, vTexCoord).rgb;
    vec3 metallicRoughness = texture(texMetallicRoughness, vTexCoord).rgb;

    vec3 normal = normalize(vNormal * sampledNormal);
    
    // Halflambert Diffuse shading.
    const vec3 lightDir = normalize(lights.directionalLight.direction);
    float diffuseStrength = dot(normal, -lightDir);
    diffuseStrength = diffuseStrength * 0.5 + 0.5;
    diffuseStrength = clamp(diffuseStrength, 0, 1);
    vec3 diffuse = baseColor * diffuseStrength * lights.directionalLight.lightColor;

    float alpha = texture(texAlbedo, vTexCoord).a;
    if (alpha <= 0.1f)
        discard; // The discard helps a bit with the current transparency issue, although it should still be fixed properly later on.

    finalColor = diffuse + lights.directionalLight.ambientColor;
    outColor = vec4(finalColor, alpha);
}
