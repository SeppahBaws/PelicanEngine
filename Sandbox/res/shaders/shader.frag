#version 450

struct DirectionalLight
{
    vec3 direction;
    vec3 lightColor;
    vec3 ambientColor;
};

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;

layout(location = 0) out vec4 fragColor;


layout(set = 0, binding = 1) uniform LightInformation
{
    DirectionalLight directionalLight;
} lights;

layout(push_constant) uniform PushConstants
{
    vec3 eyePos;
} pushConstants;

layout(binding = 2) uniform sampler2D texAlbedo;
layout(binding = 3) uniform sampler2D texNormal;
layout(binding = 4) uniform sampler2D texMetallicRoughness;
layout(binding = 5) uniform sampler2D texAmbientOcclusion;

float CalculateFresnel(vec3 N, vec3 V, float fPow)
{
    return pow((1 - clamp(abs(dot(N, V)), 0, 1)), fPow);
}

vec3 CalculateNormal(vec3 sampledNormal)
{
    // TODO: fix sampledNormal integration.

    // if (length(vTangent) == 0.0f)
        return vNormal;

    // vec3 binormal = normalize(cross(vTangent, vNormal));
    // mat3 localAxis = mat3(binormal, vTangent, vNormal);

    // sampledNormal = (2.0f * sampledNormal) - 1.0f;

    // return normalize(vNormal * sampledNormal);
}

void main()
{
    vec3 finalColor;

    vec3 baseColor = texture(texAlbedo, vTexCoord).rgb;
    vec3 sampledNormal = texture(texNormal, vTexCoord).rgb;
    vec3 metallicRoughness = texture(texMetallicRoughness, vTexCoord).rgb;
    float ao = texture(texAmbientOcclusion, vTexCoord).r;

    // Alpha discard.
    float alpha = texture(texAlbedo, vTexCoord).a;
    if (alpha <= 0.1f)
        discard;

    vec3 N = CalculateNormal(sampledNormal);
    vec3 V = normalize(pushConstants.eyePos - vPosition); // View direction

    float fresnel = CalculateFresnel(N, V, 5);
    finalColor = vec3(fresnel);

    fragColor = vec4(finalColor, alpha);
}
