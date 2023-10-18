#include "Utility.hlsli"

#define TG_SIZE 256

StructuredBuffer<uint> u_materialCounts : register(t0, space0);
RWStructuredBuffer<uint> u_indirectArgsBuffer : register(u1, space0);

struct Data
{
    uint materialCount;
};

[[vk::push_constant]] Data u_data;

[numthreads(32, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    if (threadId.x >= u_data.materialCount)
    {
        return;
    }
    
    const uint argsIndex = threadId.x * 3;
 
    u_indirectArgsBuffer[argsIndex] = DivideRoundUp(max(u_materialCounts[threadId.x], 1), TG_SIZE);
    u_indirectArgsBuffer[argsIndex + 1] = 1;
    u_indirectArgsBuffer[argsIndex + 2] = 1;
}