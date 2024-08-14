#include "PushConstant.hlsli"
#include "MeshShaderCommon.hlsli"
#include "MeshShaderCullCommon.hlsli"

struct VertexOutput
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
};

[numthreads(NUM_MS_THREADS, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
            in payload MeshAmplificationPayload payload,
            out indices uint3 tris[NUM_MAX_OUT_TRIS],
            out vertices VertexOutput vertices[NUM_MAX_OUT_VERTS],
            out primitives DefaultPrimitiveOutput primitives[NUM_MAX_OUT_TRIS])
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const PrimitiveDrawData drawData = constants.gpuScene.primitiveDrawDataBuffer.Load(payload.drawId);    
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
        float4x4 skinningMatrix = IDENTITY_MATRIX;
        if (drawData.isAnimated)
        {
            skinningMatrix = GetSkinningMatrix(mesh, vertexIndex, drawData.boneOffset, constants.gpuScene.bonesBuffer);
        }

        const float3 skinnedPosition = mul(skinningMatrix, float4(mesh.vertexPositionsBuffer.Load(vertexIndex), 1.f)).xyz;
        float4 position = mul(viewData.viewProjection, float4(drawData.transform.GetWorldPosition(skinnedPosition), 1.f));

        SetupCullingPositions(groupThreadId, position, viewData.renderSize);

        vertices[groupThreadId].position = TransformClipPosition(position);
        vertices[groupThreadId].objectId = drawData.entityId;
    }

    GroupMemoryBarrierWithGroupSync();

    if (groupThreadId < triCount)
    {
        const uint primitive = mesh.meshletDataBuffer.Load(meshlet.GetIndexOffset() + groupThreadId);
        const uint3 indices = UnpackPrimitive(primitive);        

        tris[groupThreadId] = indices;
        primitives[groupThreadId].cullPrimitive = IsPrimitiveCulled(indices);
    }
}

struct ColorOutput
{
    [[vt::r32ui]] uint objectId : SV_Target;
    [[vt::d32f]];
};

ColorOutput MainPS(VertexOutput input)
{
    ColorOutput output;
    output.objectId = input.objectId;

    return output;
}