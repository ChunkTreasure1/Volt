#include "Structures.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Common.hlsli"
#include "MeshletHelpers.hlsli"

struct Constants
{
    TTexture<uint> visibilityBuffer;
    TypedBuffer<ObjectDrawData> objectDrawData;
    TypedBuffer<Meshlet> meshletsBuffer;
    RWTypedBuffer<uint> materialCountsBuffer;

    uint2 renderSize;
};

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId.x >= constants.renderSize.x || threadId.y >= constants.renderSize.y)
    {
        return;
    }
    
    const uint pixelValue = constants.visibilityBuffer.Load2D(int3(threadId.xy, 0));
    
    if (pixelValue == UINT32_MAX)
    {
        return;
    }
    
    const Meshlet meshlet = constants.meshletsBuffer.Load(UnpackMeshletID(pixelValue));
    const ObjectDrawData objectData = constants.objectDrawData.Load(meshlet.objectId);
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
    
    constants.materialCountsBuffer.InterlockedAdd(objectData.materialId, 1);
}