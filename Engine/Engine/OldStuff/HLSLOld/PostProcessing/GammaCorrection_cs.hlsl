#include "Common.hlsli"
#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);

float3 LinearTosRGB(in float3 color)
{
    float3 x = color * 12.92f;
    float3 y = 1.055f * pow(saturate(color), 1.0f / 2.4f) - 0.055f;

    float3 clr = color;
    clr.r = color.r < 0.0031308f ? x.r : y.r;
    clr.g = color.g < 0.0031308f ? x.g : y.g;
    clr.b = color.b < 0.0031308f ? x.b : y.b;

    return clr;
}

[numthreads(8, 8, 1)]
void main(uint2 threadId : SV_DispatchThreadID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = threadId;
    
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float3 srcColor = o_resultTexture[pixelCoords].rgb;
    o_resultTexture[pixelCoords] = float4(LinearTosRGB(srcColor), 1.f);
}