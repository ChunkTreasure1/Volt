#include "Utility.hlsli"
#include "Resources.hlsli"

#define TG_SIZE 256

struct Constants
{
    vt::TypedBuffer<uint> materialCounts;
    vt::RWTypedBuffer<uint> indirectArgsBuffer;
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
    const uint materialCount = constants.materialCounts.Load(threadId.x);
    
    constants.indirectArgsBuffer.Store(argsIndex, materialCount > 0 ? DivideRoundUp(materialCount, TG_SIZE) : 0);
    constants.indirectArgsBuffer.Store(argsIndex + 1, 1);
    constants.indirectArgsBuffer.Store(argsIndex + 2, 1);
}