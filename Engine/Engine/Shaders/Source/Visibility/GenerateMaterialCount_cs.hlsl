#include "Structures.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Common.hlsli"
#include "MeshletHelpers.hlsli"

struct Constants
{
    TextureT<uint> visibilityBuffer;
    TypedBuffer<ObjectDrawData> objectDrawData;
    TypedBuffer<Meshlet> meshletsBuffer;
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