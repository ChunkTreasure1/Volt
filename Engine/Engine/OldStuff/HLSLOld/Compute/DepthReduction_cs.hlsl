#include "Defines.hlsli"
#include "SamplerStates.hlsli"

RWTexture2D<float> u_targetDepth : register(u0, SPACE_OTHER);
Texture2D<float> u_srcDepth : register(t1, SPACE_OTHER);

struct PushConstants
{
    float2 targetSize;
};

[[vk::push_constant]] PushConstants u_pushConstants;

[numthreads(32, 32, 1)]
void main(uint2 threadId : SV_DispatchThreadID)
{
    float depth = u_srcDepth.SampleLevel(u_reduceSampler, (float2(threadId) + 0.5f) / u_pushConstants.targetSize, 0.f).x;
    u_targetDepth[threadId] = depth;
}