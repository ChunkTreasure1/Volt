#include "Structures.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTypedBuffer<uint> countsBuffer;
    RWTypedBuffer<uint> drawIndexToObjectId;
    RWTypedBuffer<uint> drawIndexToMeshletId;
    
    TypedBuffer<IndirectGPUCommand> indirectCommands;
    
    uint meshletCount;
};

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    Constants constants = GetConstants<Constants>();
    
    const uint globalId = threadId.x;
    if (globalId >= constants.meshletCount)
    {
        return;
    }

    const uint objectId = constants.indirectCommands.Load(globalId).objectId;
    const uint meshletId = constants.indirectCommands.Load(globalId).meshletId;
    
    uint drawIndex;
    constants.countsBuffer.InterlockedAdd(0, 1, drawIndex);
    constants.drawIndexToObjectId.Store(drawIndex, objectId);
    constants.drawIndexToMeshletId.Store(drawIndex, meshletId);
}