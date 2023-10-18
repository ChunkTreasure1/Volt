#include "Buffers.hlsli"

struct PushConstants
{
    uint size;
};

[[vk::push_constant]] PushConstants u_pushConstants;

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    if (threadId.x >= u_pushConstants.size)
    {
        return;
    }
    
    u_indirectCounts.Store(threadId.x * 4, 0);
}