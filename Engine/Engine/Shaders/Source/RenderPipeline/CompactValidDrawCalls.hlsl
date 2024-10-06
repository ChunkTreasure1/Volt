#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Bitwise.hlsli"

struct Constants
{
    vt::UniformRWTypedBuffer<uint> validPrimitiveDrawData;
    vt::UniformTypedBuffer<PrimitiveDrawData> primitiveDrawData;

    uint primitiveDrawDataCount;
};

[numthreads(64, 1, 1)]
void MainCS(uint dispatchThreadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();

    if (dispatchThreadId >= constants.primitiveDrawDataCount)
    {
        return;
    }

    const PrimitiveDrawData primitiveDrawData = constants.primitiveDrawData.Load(dispatchThreadId);
    
    if (IsBitSet(primitiveDrawData.flags, PrimitiveFlags::Valid))
    {
        uint index;
        constants.validPrimitiveDrawData.InterlockedAdd(0, 1, index);
        constants.validPrimitiveDrawData.Store(index + 1, dispatchThreadId);
    }
};