#include "PushConstant.hlsli"
#include "MeshShaderCommon.hlsli"

#include "MeshletHelpers.hlsli"

struct VertexOutput
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
    uint meshletId : MESHLET_ID;
};

struct PrimitiveOutput
{
    uint primitiveID : SV_PrimitiveID;
    bool cullPrimitive : SV_CullPrimitive;
};

[numthreads(NUM_MS_THREADS, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
            in payload MeshAmplificationPayload payload,
            out indices uint3 tris[NUM_MAX_OUT_TRIS],
            out vertices VertexOutput vertices[NUM_MAX_OUT_VERTS],
            out primitives PrimitiveOutput primitives[NUM_MAX_OUT_TRIS])
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const ObjectDrawData drawData = constants.gpuScene.objectDrawDataBuffer.Load(payload.drawId);    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    uint meshletIndex = payload.meshletIndices[groupId];
    if (meshletIndex >= mesh.meshletCount)
    {
        return;
    }

    const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + meshletIndex);
    const uint vertexCount = meshlet.GetVertexCount();
    const uint triCount = meshlet.GetTriangleCount();

    SetMeshOutputCounts(vertexCount, triCount);

    if (groupThreadId < vertexCount)
    { 
        const uint vertexIndex = mesh.meshletDataBuffer[meshlet.GetVertexOffset() + groupThreadId] + mesh.vertexStartOffset;

        float4 position = TransformClipPosition(mul(viewData.viewProjection, float4(drawData.transform.GetWorldPosition(mesh.vertexPositionsBuffer.Load(vertexIndex)), 1.f)));

        SetupCullingPositions(groupThreadId, position, viewData.renderSize);

        vertices[groupThreadId].position = position;
        vertices[groupThreadId].objectId = payload.drawId;
        vertices[groupThreadId].meshletId = meshletIndex;
    } 

    GroupMemoryBarrierWithGroupSync();

    if (groupThreadId < triCount)
    {
        const uint primitive = mesh.meshletDataBuffer.Load(meshlet.GetIndexOffset() + groupThreadId);
        const uint3 indices = UnpackPrimitive(primitive);        

        tris[groupThreadId] = indices;
        primitives[groupThreadId].primitiveID = groupThreadId;
        primitives[groupThreadId].cullPrimitive = IsPrimitiveCulled(indices);
    }
}

struct ColorOutput
{
    [[vt::rg32ui]] uint2 visibilityId : SV_Target;
    [[vt::d32f]];
};

struct VertexInput
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
    uint meshletId : MESHLET_ID;
    uint primitiveID : SV_PrimitiveID;
};

ColorOutput MainPS(VertexInput input)
{
    ColorOutput output;
    output.visibilityId = uint2(input.objectId, PackMeshletAndIndex(input.meshletId, input.primitiveID));

    return output;
}