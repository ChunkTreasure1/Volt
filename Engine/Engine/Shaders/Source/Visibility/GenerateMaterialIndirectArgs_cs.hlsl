#include "Utility.hlsli"
#include "Resources.hlsli"

#define TG_SIZE 256

struct Constants
{
    TypedBuffer<uint> materialCounts;
    RWTypedBuffer<uint> indirectArgsBuffer;
    uint materialCount;
};

[numthreads(32, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId.x >= constants.materialCount)
    {
        return;
    }
    
    const uint argsIndex = threadId.x * 3;
 
    constants.indirectArgsBuffer.Store(argsIndex, DivideRoundUp(max(constants.materialCounts.Load(threadId.x), 1), TG_SIZE));
    constants.indirectArgsBuffer.Store(argsIndex + 1, 1);
    constants.indirectArgsBuffer.Store(argsIndex + 2, 1);
}