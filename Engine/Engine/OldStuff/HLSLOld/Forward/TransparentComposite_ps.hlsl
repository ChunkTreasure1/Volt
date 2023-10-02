#include "Vertex.hlsli"

Texture2D<float4> u_accumulation : register(t0, SPACE_MATERIAL);
Texture2D<float4> u_revealage : register(t1, SPACE_MATERIAL);

static const float EPSILON = 0.00001f;

bool IsApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

float max3(float3 val)
{
    return max(max(val.r, val.g), val.b);
}

float4 main(DefaultFullscreenTriangleVertex input) : SV_Target0
{
    int2 coords = int2(input.position.xy);
    
    const float revealage = u_revealage.Load(int3(coords, 0)).r;
    if (IsApproximatelyEqual(revealage, 1.f))
    {
        discard;
    }

    float4 accumulation = u_accumulation.Load(int3(coords, 0));
    
    if (isinf(max3(accumulation.rgb)))
    {
        accumulation.rgb = accumulation.a;
    }

    float3 avgColor = accumulation.rgb / max(accumulation.a, EPSILON);
    return float4(avgColor, 1.f - revealage);
}