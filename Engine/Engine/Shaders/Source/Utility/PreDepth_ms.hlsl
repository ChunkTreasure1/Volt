#include "Defines.hlsli"
#include "GPUScene.hlsli"
#include "DrawContext.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"

#define THREAD_GROUP_SIZE 32

struct Constants
{
    TypedBuffer<GPUScene> gpuScene;
    TypedBuffer<DrawContext> drawContext;
    
    TypedBuffer<CameraData> cameraData; // #TODO_Ivar: Should be uniform buffer
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
};

struct Payload
{
    uint baseId;
    uint subIds[THREAD_GROUP_SIZE];
};

[outputtopology("triangle")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(in payload Payload inPayload, out indices uint3 outTriangles[1], out vertices VertexOutput outVertices[3], uint dispatchThreadID : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    const GPUScene scene = constants.gpuScene.Load(0);
    const CameraData cameraData = constants.cameraData.Load(0);
    const DrawContext context = constants.drawContext.Load(0);
    
    const uint meshletIndex = inPayload.baseId + inPayload.subIds[groupThreadId];
    const uint meshletId = context.drawIndexToMeshletId.Load(meshletIndex);
    const uint objectId = context.drawIndexToObjectId.Load(drawIndex);
   
    const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    const Meshlet meshlet = mesh.meshletsBuffer.Load(meshletId);
    
    SetMeshOutputCounts(3, 1);
    
    for (uint i = 0; i < 3; i++)
    {
        outVertices[i].position = m
    }
        outTriangles[0] = uint3(0, 1, 2);
}