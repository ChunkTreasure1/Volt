#include "Structures.hlsli"

struct DrawCullData
{
    uint drawCallCount;
};

[[vk::push_constant]] DrawCullData u_cullData;

RWStructuredBuffer<uint> u_countsBuffer : register(u0, space0);
StructuredBuffer<IndirectGPUCommand> u_indirectCommands : register(t1, space0);
RWStructuredBuffer<uint> u_drawToInstanceOffset : register(u2, space0);
RWStructuredBuffer<uint> u_instanceOffsetToObjectID : register(u3, space0);

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const uint globalId = threadId.x;
    if (globalId >= u_cullData.drawCallCount)
    {
        return;
    }

    const uint objectId = u_indirectCommands[globalId].objectId;
    const uint instanceCount = u_indirectCommands[globalId].instanceCount;
  
    uint drawIndex;
    InterlockedAdd(u_countsBuffer[0], 1, drawIndex);
    
    uint instanceOffset;
    InterlockedAdd(u_countsBuffer[1], instanceCount, instanceOffset);
  
    u_drawToInstanceOffset[drawIndex] = drawIndex;
  
    for (uint i = 0; i < instanceCount; i++)
    {
        u_instanceOffsetToObjectID[instanceOffset + i] = objectId + i;
    }
}