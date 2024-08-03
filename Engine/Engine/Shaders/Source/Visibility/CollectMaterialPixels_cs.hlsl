#include "Common.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "MeshletHelpers.hlsli"

struct Constants
{
    GPUScene gpuScene;

    vt::UniformTex2D<uint2> visibilityBuffer;
    vt::UniformTypedBuffer<uint> materialStartBuffer;
    
    vt::UniformRWTypedBuffer<uint> currentMaterialCountBuffer;
    vt::UniformRWTypedBuffer<uint2> pixelCollectionBuffer;

    uint2 renderSize;
};

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (any(threadId.xy >= constants.renderSize))
    {
        return;
    }
    
    const uint2 pixelValue = constants.visibilityBuffer.Load(int3(threadId.xy, 0));
    
    if (pixelValue.x == UINT32_MAX)
    {
        return;
    }
    
    const PrimitiveDrawData objectData = constants.gpuScene.primitiveDrawDataBuffer.Load(pixelValue.x);
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
    
    uint materialStartIndex = constants.materialStartBuffer.Load(objectData.materialId);

    uint currentIndex;
    constants.currentMaterialCountBuffer.InterlockedAdd(objectData.materialId, 1, currentIndex);
    constants.pixelCollectionBuffer.Store(materialStartIndex + currentIndex, threadId.xy);
}