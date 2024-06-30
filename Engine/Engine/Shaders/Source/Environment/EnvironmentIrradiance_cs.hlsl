RWTexture2DArray<float3> o_output;
TextureCube<float3> u_input;
SamplerState u_linearSampler;

static const float m_pi = 3.1415926535897932384626433832795f;
static const float m_twoPi = m_pi * 2.0f;
static const float m_epsilon = 0.00001f;

static const uint m_numSamples = 1024;
static const float m_invNumSamples = 1.0f / float(m_numSamples);

static const uint m_numMips = 1;

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 SampleHammersley(uint i, uint samples)
{
    float invSamples = 1.f / float(samples);
    return float2(i * invSamples, RadicalInverse_VdC(i));
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
    float3 N = GetCubeMapTexCoord(dispatchId);

    float3 S, T;
    ComputeBasisVector(N, S, T);

    uint samples = 64 * m_numSamples;

    float3 irradiance = 0.f;
    for (uint i = 0; i < samples; ++i)
    {
        float2 sample = SampleHammersley(i, samples);
        float3 Li = TangentToWorld(SampleHemisphere(sample.x, sample.y), N, S, T);
        float cosTheta = max(0.f, dot(Li, N));

        irradiance += 2.f * u_input.SampleLevel(u_linearSampler, Li, 0).rgb * cosTheta;
    }

    irradiance /= float(samples);
    o_output[dispatchId] = irradiance;
}