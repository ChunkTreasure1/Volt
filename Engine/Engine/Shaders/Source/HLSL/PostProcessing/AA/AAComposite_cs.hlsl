#include "Defines.hlsli"

Texture2D<float4> u_tempTexture : register(t0, SPACE_OTHER);
RWTexture2D<float4> o_result : register(u1, SPACE_OTHER);

//float3 InverseTonemap_ACES(float3 x) 
//{
//    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
//    const float a = 2.51;
//    const float b = 0.03;
//    const float c = 2.43;
//    const float d = 0.59;
//    const float e = 0.14;
//    return (-d * x + b - sqrt(-1.0127 * x*x + 1.3702 * x + 0.0009)) / (2.0 * (c*x - a));
//}

[numthreads(8, 8, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixCoord = dispatchThreadID.xy;

    o_result[pixCoord] = float4(u_tempTexture.Load(uint3(pixCoord, 0)).rgb, 1);
}