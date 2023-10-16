#include "Common.hlsli"
#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

Texture2D<float> u_depthTexture : register(t0, SPACE_OTHER);
RWTexture2D<float> u_outputs[8] : register(t1, SPACE_OTHER);

static const float2 UV_OFFSETS[2] = { float2(0.5f, 0.5), float2(0.5f, 2.5f) };

struct PushConstants
{
    uint uvOffsetIndex;
};

[[vk::push_constant]] PushConstants u_pushConstants;

[numthreads(8, 8, 1)]
void main(uint2 threadId : SV_DispatchThreadID)
{
    float outputWidth, outputHeight;
    u_outputs[0].GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = threadId;
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float2 texCoords = float2(float(threadId.x) / outputWidth, 1.f - (float(threadId.y) / outputHeight));

    float2 uv = floor(float2(threadId)) * 4.f + (UV_OFFSETS[u_pushConstants.uvOffsetIndex] + 0.5f);
    uv.xy *= u_pass

}