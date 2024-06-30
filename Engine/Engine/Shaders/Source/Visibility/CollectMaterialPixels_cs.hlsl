#include "Common.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "MeshletHelpers.hlsli"

struct Constants
{
    GPUScene gpuScene;

    TTexture<uint> visibilityBuffer;
    TypedBuffer<uint> materialStartBuffer;
    
    RWTypedBuffer<uint> currentMaterialCountBuffer;
    RWTypedBuffer<uint2> pixelCollectionBuffer;

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
    
    const uint pixelValue = constants.visibilityBuffer.Load2D(int3(threadId.xy, 0));
    
    if (pixelValue == UINT32_MAX)
    {
        return;
    }
    
    const Meshlet meshlet = constants.gpuScene.meshletsBuffer.Load(UnpackMeshletID(pixelValue));
    const ObjectDrawData objectData = constants.gpuScene.objectDrawDataBuffer.Load(meshlet.objectId);
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
    
    uint materialStartIndex = constants.materialStartBuffer.Load(objectData.materialId);

    uint currentIndex;
    constants.currentMaterialCountBuffer.InterlockedAdd(objectData.materialId, 1, currentIndex);
    constants.pixelCollectionBuffer.Store(materialStartIndex + currentIndex, threadId.xy);
}