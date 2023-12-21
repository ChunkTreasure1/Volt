#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

#define ITERATIONS_PER_GROUP 2
#define THREAD_GROUP_SIZE 32

#define MESHLET_ID_BITS 24u
#define MESHLET_PRIMITIVE_BITS 8u
#define MESHLET_ID_MASK ((1u << MESHLET_ID_BITS) - 1u)
#define MESHLET_PRIMITIVE_MASK ((1u << MESHLET_PRIMITIVE_BITS) - 1u)

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
groupshared Meshlet m_meshlet;
groupshared GPUMesh m_mesh;
groupshared uint m_meshletIndex;

[numthreads(64, 1, 1)]
void main(uint2 groupId : SV_GroupID, uint groupThreadId : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (groupThreadId == 0)
    {
        m_meshletIndex = constants.survivingMeshlets.Load(groupId.x);
        m_meshlet = constants.gpuMeshlets.Load(m_meshletIndex);
        m_mesh = constants.gpuMeshes.Load(m_meshlet.meshId);
    }

    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadId >= m_meshlet.triangleCount)
    {
        return;
    }
    
    //const ObjectDrawData objectData = constants.objectDrawDataBuffer.Load(meshlet.objectId);
    
    const uint triangleId = groupThreadId * 3;
    const uint index0 = m_mesh.meshletIndexBuffer.Load(m_mesh.meshletIndexStartOffset + m_meshlet.triangleOffset + triangleId + 0);
    const uint index1 = m_mesh.meshletIndexBuffer.Load(m_mesh.meshletIndexStartOffset + m_meshlet.triangleOffset + triangleId + 1);
    const uint index2 = m_mesh.meshletIndexBuffer.Load(m_mesh.meshletIndexStartOffset + m_meshlet.triangleOffset + triangleId + 2);
    
    if (groupThreadId == 0)
    {
        constants.drawCommand.InterlockedAdd(0, m_meshlet.triangleCount * 3, m_indexOffset);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    const uint baseOffset = m_indexOffset + triangleId;
    constants.indexBuffer.Store(baseOffset + 0, (m_meshletIndex << MESHLET_PRIMITIVE_BITS) | ((index0) & MESHLET_PRIMITIVE_MASK));
    constants.indexBuffer.Store(baseOffset + 1, (m_meshletIndex << MESHLET_PRIMITIVE_BITS) | ((index1) & MESHLET_PRIMITIVE_MASK));
    constants.indexBuffer.Store(baseOffset + 2, (m_meshletIndex << MESHLET_PRIMITIVE_BITS) | ((index2) & MESHLET_PRIMITIVE_MASK));

}