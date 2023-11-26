#include "Common.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"

struct Constants
{
    TextureT<uint2> visibilityBuffer;
    TypedBuffer<ObjectDrawData> objectDrawData;
    TypedBuffer<uint> materialStartBuffer;
    
    RWTypedBuffer<uint> currentMaterialCountBuffer;
    RWTypedBuffer<uint2> pixelCollectionBuffer;
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
    
    const ObjectDrawData objectData = constants.objectDrawData.Load(pixelValue.x);
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
    
    uint materialStartIndex = constants.materialStartBuffer.Load(objectData.materialId);

    uint currentIndex;
    constants.currentMaterialCountBuffer.InterlockedAdd(objectData.materialId, 1, currentIndex);
    constants.pixelCollectionBuffer.Store(materialStartIndex + currentIndex, threadId.xy);
}