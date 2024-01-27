#include "Defines.hlsli"

RWTexture2DArray<float3> o_output : register(u0, space0);
TextureCube<float3> u_input : register(t1, space0);
SamplerState u_linearSampler : register(s2, space0);

static const float m_pi = 3.1415926535897932384626433832795f;
static const float m_twoPi = m_pi * 2.0f;
static const float m_epsilon = 0.00001f;

static const uint m_numSamples = 1024;
static const float m_invNumSamples = 1.0f / float(m_numSamples);

static const uint m_numMips = 1;

struct PushConstants
{
    float roughness;
};

PUSH_CONSTANT(PushConstants, u_pushConstants);

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 SampleHammersley(uint i)
{
    return float2(i * m_invNumSamples, RadicalInverse_VdC(i));
}

float3 SampleGGX(float u1, float u2, float roughness)
{
    float alpha = roughness * roughness;

    float cosTheta = sqrt((1.f - u2) / (1.f + (alpha * alpha - 1.f) * u2));
    float sinTheta = sqrt(1.f - cosTheta * cosTheta);
    float phi = m_twoPi * u1;

	// Convert to Cartesian upon return
    return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

float NDFGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.f) + 1.f;

    return alphaSq / (m_pi * denom * denom);
}

float3 GetCubeMapTexCoord(uint3 dispatchId)
{
    uint2 texSize;
    uint elements;

    o_output.GetDimensions(texSize.x, texSize.y, elements);

    float2 ST = dispatchId.xy / float2(texSize.x, texSize.y);
    float2 UV = 2.f * float2(ST.x, 1.f - ST.y) - 1.f;

    float3 result = 0.f;
    switch (dispatchId.z)
    {
        case 0:
            result = float3(1.f, UV.y, -UV.x);
            break;
        case 1:
            result = float3(-1.f, UV.y, UV.x);
            break;
        case 2:
            result = float3(UV.x, 1.f, -UV.y);
            break;
        case 3:
            result = float3(UV.x, -1.f, UV.y);
            break;
        case 4:
            result = float3(UV.x, UV.y, 1.f);
            break;
        case 5:
            result = float3(-UV.x, UV.y, -1.f);
            break;
    }

    return normalize(result);
}

void ComputeBasisVector(const float3 normal, out float3 S, out float3 T)
{
    T = cross(normal, float3(0.f, 1.f, 0.f));
    T = lerp(cross(normal, float3(1.f, 0.f, 0.f)), T, step(m_epsilon, dot(T, T)));

    T = normalize(T);
    S = normalize(cross(normal, T));
}

float3 TangentToWorld(const float3 V, const float3 normal, const float3 S, const float3 T)
{
    return S * V.x + T * V.y + normal * V.z;
}

float3 SampleHemisphere(float u1, float u2)
{
    const float u1p = sqrt(max(0.f, 1.f - u1 * u1));
    return float3(cos(m_twoPi * u2) * u1p, sin(m_twoPi * u2) * u1p, u1);
}

[numthreads(32, 32, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    uint2 outputSize;
    uint outputElements;

    o_output.GetDimensions(outputSize.x, outputSize.y, outputElements);

    if (dispatchId.x >= outputSize.x || dispatchId.y >= outputSize.y)
    {
        return;
    }

    float2 inputSize;
    float inputElements;
    u_input.GetDimensions(0, inputSize.x, inputSize.y, inputElements);

    float wt = 4.f * m_pi / (6.f * inputSize.x * inputSize.y);
    float3 N = GetCubeMapTexCoord(dispatchId);
    float3 Lo = N;

    float3 S, T;
    ComputeBasisVector(N, S, T);

    float3 color = 0.f;
    float weight = 0.f;

    for (uint i = 0; i < m_numSamples; i++)
    {
        float2 u = SampleHammersley(i);
        float3 Lh = TangentToWorld(SampleGGX(u.x, u.y, u_pushConstants.roughness), N, S, T);

        float3 Li = 2.f * dot(Lo, Lh) * Lh - Lo;
        float cosLi = dot(N, Li);
        if (cosLi > 0.f)
        {
            float cosLh = max(dot(N, Lh), 0.f);
            float pdf = NDFGGX(cosLh, u_pushConstants.roughness) * 0.25f;
            float ws = 1.f / (m_numSamples * pdf);

            float mipLevel = max(0.5f * log2(ws / wt) + 1.f, 0.f);
            color += u_input.SampleLevel(u_linearSampler, Li, mipLevel).rgb * cosLi;
            weight += cosLi;
        }
    }

    color /= weight;
    o_output[dispatchId] = color;
}