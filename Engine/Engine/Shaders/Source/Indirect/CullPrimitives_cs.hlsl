#include "Structures.hlsli"
#include "Resources.hlsli"

#include "ComputeUtilities.hlsli"
#include "GPUScene.hlsli"
#include "MeshletHelpers.hlsli"

#define ITERATIONS_PER_GROUP 2
#define THREAD_GROUP_SIZE 32

struct Constants
{
    UniformRWTypedBuffer<uint> indexBuffer;
    UniformRWTypedBuffer<uint> drawCommand;
  
    UniformTypedBuffer<uint> survivingMeshlets;
    UniformTypedBuffer<uint> survivingMeshletCount;
    
    GPUScene gpuScene;

    float4x4 viewMatrix;
    float4x4 projectionMatrix;

    float2 renderSize;
};

[numthreads(64, 1, 1)]
void main(uint3 gid : SV_GroupID, uint groupThreadId : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    uint groupId = UnwrapDispatchGroupId(gid);
    
    const uint meshletIndex = constants.survivingMeshlets.Load(groupId);
    const Meshlet meshlet = constants.gpuScene.meshletsBuffer.Load(meshletIndex);
    
    if (groupThreadId >= meshlet.triangleCount)
    {
        return;
    }
    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(meshlet.meshId);
    const ObjectDrawData objectData = constants.gpuScene.objectDrawDataBuffer.Load(meshlet.objectId);
    
    const uint triangleId = groupThreadId * 3;
    const uint index0 = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + triangleId + 0);
    const uint index1 = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + triangleId + 1);
    const uint index2 = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + triangleId + 2);
    
    const uint vIndex0 = index0 + meshlet.vertexOffset + mesh.vertexStartOffset;
    const uint vIndex1 = index1 + meshlet.vertexOffset + mesh.vertexStartOffset;
    const uint vIndex2 = index2 + meshlet.vertexOffset + mesh.vertexStartOffset;
    
    float3 positions[3];
    float3 vertexClip[3];
   
    positions[0] = mesh.vertexPositionsBuffer.Load(vIndex0).position;
    positions[1] = mesh.vertexPositionsBuffer.Load(vIndex1).position;
    positions[2] = mesh.vertexPositionsBuffer.Load(vIndex2).position;
    
    // #TODO_Ivar: switch to viewProjection
    const float4x4 mvp = mul(constants.projectionMatrix, mul(constants.viewMatrix, objectData.transform));
    
    [unroll]
    for (uint i = 0; i < 3; ++i)
    {
        float4 clipPosition = mul(mvp, float4(positions[i], 1.f));
        vertexClip[i] = float3(clipPosition.xy / clipPosition.w, clipPosition.w);
    }
    
    bool culled = false;
    
    float2 pa = vertexClip[0].xy, pb = vertexClip[1].xy, pc = vertexClip[2].xy;
    
    float2 eb = pb - pa;
    float2 ec = pc - pa;
    
    culled = culled || (eb.x * ec.y >= eb.y * ec.x);
    
    float2 bmin = (min(pa, min(pb, pc)) * 0.5f + 0.5f) * constants.renderSize;
    float2 bmax = (max(pa, max(pb, pc)) * 0.5f + 0.5f) * constants.renderSize;
    float sbprec = 1.f / 256.f;
    
    culled = culled || (round(bmin.x - sbprec) == round(bmax.x + sbprec) || round(bmin.y - sbprec) == round(bmax.y + sbprec));
    culled = culled && (vertexClip[0].z > 0.f && vertexClip[1].z > 0.f && vertexClip[2].z > 0.f);
    
    const bool visible = !culled;
    
    uint visibleCount = WaveActiveCountBits(visible);
    uint lanePrefix = WavePrefixCountBits(visible);
    
    uint waveOffset;
    if (WaveIsFirstLane())
    {
        constants.drawCommand.InterlockedAdd(0, visibleCount * 3, waveOffset);
    }
    
    waveOffset = WaveReadLaneFirst(waveOffset);
    
    if (visible)
    {
        const uint baseOffset = waveOffset + lanePrefix * 3;
        constants.indexBuffer.Store(baseOffset + 0, PackMeshletAndIndex(meshletIndex, triangleId + 0));
        constants.indexBuffer.Store(baseOffset + 1, PackMeshletAndIndex(meshletIndex, triangleId + 1));
        constants.indexBuffer.Store(baseOffset + 2, PackMeshletAndIndex(meshletIndex, triangleId + 2));
    }
}