#include "Structures.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Common.hlsli"

struct Constants
{
    TextureT<uint2> visibilityBuffer;
    TypedBuffer<ObjectDrawData> objectDrawData;
    RWTypedBuffer<uint> materialCountsBuffer;
};

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    uint2 size;
    constants.visibilityBuffer.GetDimensions(size.x, size.y);
    
    if (threadId.x >= size.x || threadId.y >= size.y)
    {
        return;
    }
    
    const uint2 pixelValue = constants.visibilityBuffer.Load2D(int3(threadId.xy, 0));
    
    if (pixelValue.x == UINT32_MAX)
    {
        return;
    }
    
    ObjectDrawData objectData = constants.objectDrawData.Load(pixelValue.x);
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
    
    constants.materialCountsBuffer.InterlockedAdd(objectData.materialId, 1);
}