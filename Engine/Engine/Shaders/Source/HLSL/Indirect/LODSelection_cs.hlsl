#include "Buffers.hlsli"
#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"

struct SelectionData
{
    uint drawCallCount;
    float3 cameraPosition;
    
    float lodBase, lodStep;
};

[[vk::push_constant]] SelectionData u_selectionData;

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const uint globalId = threadId.x;
    if (globalId < u_selectionData.drawCallCount)
    {
        const uint objectId = u_indirectBatches[globalId].objectId;
        const ObjectData objectData = u_objectData[objectId];
        
        GPUMesh mesh = u_meshes[objectData.meshIndex];
        
        float dist = distance(u_selectionData.cameraPosition, objectData.boundingSphereCenter);
        float lodIndexF = log2(dist / u_selectionData.lodBase) / log2(u_selectionData.lodStep);
        uint lodIndex = min(uint(max(lodIndexF + 1.f, 0)), mesh.lodCount - 1);

        GPUMeshLOD lod = mesh.lods[lodIndex];
        
        u_indirectBatches[globalId].firstIndex = lod.indexOffset;
        u_indirectBatches[globalId].indexCount = lod.indexCount;
    }
}