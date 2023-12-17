#include "Defines.hlsli"
#include "GPUScene.hlsli"
#include "DrawContext.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"

#define THREAD_GROUP_SIZE 32
#define MAX_VERTEX_COUNT 64
#define MAX_PRIMITIVE_COUNT 124

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

static const uint m_threadVertexCount = (MAX_VERTEX_COUNT + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
static const uint m_threadPrimitiveCount = (3 * MAX_PRIMITIVE_COUNT + THREAD_GROUP_SIZE * 4 - 1) / (THREAD_GROUP_SIZE * 4);

[outputtopology("triangle")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(in payload Payload inPayload, out indices uint3 outTriangles[MAX_PRIMITIVE_COUNT], out vertices VertexOutput outVertices[MAX_VERTEX_COUNT], 
    uint dispatchThreadID : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID, [[vk::builtin("DrawIndex")]] uint drawIndex)
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
    
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);
    
    const uint vertexOffset = meshlet.vertexOffset + mesh.vertexStartOffset;
    
    const float4x4 transform = drawData.transform;
    const float4x4 mvp = mul(cameraData.projection, mul(cameraData.view, transform));
    
    for (uint i = 0; i < m_threadVertexCount; ++i)
    {
        const uint vertexIndex = groupThreadId + i * THREAD_GROUP_SIZE;
        
        vertexIndex = min(vertexIndex, meshlet.vertexCount - 1);
        const float3 vertexPosition = mesh.vertexPositionsBuffer.Load(vertexIndex);
        
        outVertices[i].position = mul(mvp, float4(vertexPosition, 1.f));
    }
    
    uint primitiveCount = meshlet.triangleCount * 3;
    uint primitiveOffset = meshlet.triangleOffset + mesh.meshletTriangleStartOffset;
    for (uint i = 0; i < primitiveCount; i += 3)
    {
        uint primIndex = groupThreadId + i * THREAD_GROUP_SIZE;
        primIndex = min(primIndex, primitiveCount);
        
        outTriangles[primIndex] = uint3(mesh.meshletIndexBuffer.Load(primitiveOffset + i + 0),
                                        mesh.meshletIndexBuffer.Load(primitiveOffset + i + 1),
                                        mesh.meshletIndexBuffer.Load(primitiveOffset + i + 2));
    }
}