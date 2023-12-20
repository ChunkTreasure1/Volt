#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

struct Constants
{
    RWTypedBuffer<uint> survivingMeshlets;
    RWTypedBuffer<uint> survivingMeshletCount;
    
    TypedBuffer<uint> meshletCount;
    TypedBuffer<uint2> meshletToObjectIdAndOffset;
    
    TypedBuffer<Meshlet> gpuMeshlets;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<CameraData> cameraData;
};

[numthreads(256, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId >= constants.meshletCount.Load(0))
    {
        return;
    }
 
    const uint2 objectIdAndOffset = constants.meshletToObjectIdAndOffset.Load(threadId);
    const ObjectDrawData objectData = constants.objectDrawDataBuffer.Load(objectIdAndOffset.x);
    
    const uint meshletIndex = objectData.meshletStartOffset + threadId - objectIdAndOffset.y;
    const Meshlet meshlet = constants.gpuMeshlets.Load(meshletIndex);
    
    uint offset;
    constants.survivingMeshletCount.InterlockedAdd(0, 1, offset);
    constants.survivingMeshlets.Store(offset, meshletIndex);
}