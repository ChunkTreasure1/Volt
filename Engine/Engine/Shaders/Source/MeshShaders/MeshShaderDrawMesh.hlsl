#include "Resources.hlsli"
#include "GPUScene.hlsli"

struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;

    uint objectId;
};

struct VertexOutput
{};

[numthreads(64, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID
            out indices uint3 tris[64],
            out vertices )
{
    const Constants constants = GetConstants<Constants>();
    const ObjectDrawData objectData = constants.gpuScene.objectDrawDataBuffer.Load(constants.objectId);
    const GPUMesh gpuMesh = constants.gpuScene.meshesBuffer.Load(objectData.meshId);
    
    const Meshlet meshlet = gpuMesh.meshletsBuffer.Load(gpuMesh.meshletStartOffset + groupId);

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);
    
    if (groupThreadId < meshlet.triangleCount)
    {
        uint i0 = gpuMesh.meshletIndexBuffer.Load(gpuMesh.meshletIndexStartOffset + meshlet.triangleOffset + 0) + meshlet.vertexOffset + gpuMesh.vertexStartOffset;
        uint i1 = gpuMesh.meshletIndexBuffer.Load(gpuMesh.meshletIndexStartOffset + meshlet.triangleOffset + 1) + meshlet.vertexOffset + gpuMesh.vertexStartOffset;
        uint i2 = gpuMesh.meshletIndexBuffer.Load(gpuMesh.meshletIndexStartOffset + meshlet.triangleOffset + 2) + meshlet.vertexOffset + gpuMesh.vertexStartOffset;

    
    }
}