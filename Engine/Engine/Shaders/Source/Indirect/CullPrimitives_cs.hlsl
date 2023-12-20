#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

#define ITERATIONS_PER_GROUP 2
#define THREAD_GROUP_SIZE 32

#define MESHLET_ID_BITS 24u
#define MESHLET_PRIMITIVE_BITS 8u
#define MESHLET_ID_MASK ((1u << MESHLET_ID_BITS) - 1u)
#define MESHLET_PRIMITIVE_ID_MASK ((1u << MESHLET_PRIMITIVE_BITS) - 1u)

struct Constants
{
    RWTypedBuffer<uint> indexBuffer;
    RWTypedBuffer<uint> drawCommand;
  
    TypedBuffer<uint> survivingMeshlets;
    TypedBuffer<uint> survivingMeshletCount;
    
    TypedBuffer<Meshlet> gpuMeshlets;
    TypedBuffer<GPUMesh> gpuMeshes;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<CameraData> cameraData;
};

groupshared uint m_indexOffset;

[numthreads(64, 1, 1)]
void main(uint threadId : SV_DispatchThreadID, uint2 groupId : SV_GroupID, uint groupThreadId : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    const uint meshletIndex = constants.survivingMeshlets.Load(groupId.x);
    const Meshlet meshlet = constants.gpuMeshlets.Load(meshletIndex);
    
    if (groupThreadId >= meshlet.triangleCount)
    {
        return;
    }
    
    const GPUMesh gpuMesh = constants.gpuMeshes.Load(meshlet.meshId);
    const ObjectDrawData objectData = constants.objectDrawDataBuffer.Load(meshlet.objectId);
    
    const uint triangleId = groupThreadId * 3;
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadId == 0)
    {
        constants.drawCommand.InterlockedAdd(0, meshlet.triangleCount * 3, m_indexOffset);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    const uint baseOffset = m_indexOffset + triangleId;
    constants.indexBuffer.Store(baseOffset + 0, (meshletIndex << MESHLET_PRIMITIVE_BITS) | ((triangleId + 0) & MESHLET_PRIMITIVE_ID_MASK));
    constants.indexBuffer.Store(baseOffset + 1, (meshletIndex << MESHLET_PRIMITIVE_BITS) | ((triangleId + 1) & MESHLET_PRIMITIVE_ID_MASK));
    constants.indexBuffer.Store(baseOffset + 2, (meshletIndex << MESHLET_PRIMITIVE_BITS) | ((triangleId + 2) & MESHLET_PRIMITIVE_ID_MASK));

}