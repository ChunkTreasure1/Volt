#include "Structures.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTypedBuffer<uint> countsBuffer;
    TypedBuffer<IndirectGPUCommand> indirectCommands;
    RWTypedBuffer<uint> drawToInstanceOffset;
    RWTypedBuffer<uint> instanceOffsetToObjectID;
    
    uint drawCallCount;
};

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    Constants constants = GetConstants<Constants>();
    
    const uint globalId = threadId.x;
    if (globalId >= constants.drawCallCount)
    {
        return;
    }

    const uint objectId = constants.indirectCommands.Load(globalId).objectId;
    const uint instanceCount = constants.indirectCommands.Load(globalId).instanceCount;
  
    uint drawIndex;
    constants.countsBuffer.InterlockedAdd(0, 1, drawIndex);
    
    uint instanceOffset;
    constants.countsBuffer.InterlockedAdd(1, instanceCount, instanceOffset);
  
    constants.drawToInstanceOffset.Store(drawIndex, drawIndex);
  
    //for (uint i = 0; i < instanceCount; i++)
    //{
    //    constants.instanceOffsetToObjectID.Store(instanceOffset + i, objectId + i);
    //}
}