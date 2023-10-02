#include "Common.hlsli"
#include "Defines.hlsli"

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);
Texture2D<float4> u_jumpFloodTexture : register(t1, SPACE_OTHER);

struct PushConstants
{
    float4 color;
};

[[vk::push_constant]] PushConstants u_pushConstants;

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
    
    float4 pixel = u_jumpFloodTexture.Load(int3(pixelCoords, 0));
    float dist = sqrt(pixel.z);
    float alpha = smoothstep(0.004f, 0.002f, dist);
    if (alpha == 0.f)
    {
        return;
    }

    o_resultTexture[pixelCoords] = float4(u_pushConstants.color.xyz, 1.f);
};