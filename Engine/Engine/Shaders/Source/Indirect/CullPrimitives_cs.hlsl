#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

#define ITERATIONS_PER_GROUP 2
#define THREAD_GROUP_SIZE 32

#define MESHLET_ID_BITS 24u
#define MESHLET_PRIMITIVE_ID_BITS 8u
#define MESHLET_ID_MASK ((1u << MESHLET_ID_BITS) - 1u)
#define MESHLET_PRIMITIVE_ID_MASK ((1u << MESHLET_PRIMITIVE_ID_BITS) - 1u)

struct Constants
{
    TypedBuffer<uint> survivingMeshlets;
    TypedBuffer<uint> survivingMeshletCount;
    
    TypedBuffer<Meshlet> gpuMeshlets;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<CameraData> cameraData;
};

[numthreads(64, 1, 1)]
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