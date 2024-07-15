#include "PushConstant.hlsli"
#include "MeshShaderCommon.hlsli"

struct PerDrawData
{
    uint drawIndex;
};

PUSH_CONSTANT(PerDrawData, u_perDrawData);

struct VertexOutput
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
};

[numthreads(NUM_MS_THREADS, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
            out indices uint3 tris[NUM_MAX_OUT_TRIS],
            out vertices VertexOutput vertices[NUM_MAX_OUT_VERTS])
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const ObjectDrawData drawData = constants.gpuScene.objectDrawDataBuffer.Load(u_perDrawData.drawIndex);    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + groupId);
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    uint dataOffset = meshlet.dataOffset;
    uint vertexOffset = dataOffset;
    uint indexOffset = dataOffset + meshlet.vertexCount;

    if (groupThreadId < meshlet.triangleCount)
    {
        const uint primitive = mesh.meshletDataBuffer.Load(indexOffset + groupThreadId);
        tris[groupThreadId] = UnpackPrimitive(primitive);
    }

    if (groupThreadId < meshlet.vertexCount)
    {
        const uint vertexIndex = mesh.meshletDataBuffer[vertexOffset + groupThreadId] + mesh.vertexStartOffset;

        float4 position = mul(viewData.viewProjection, float4(drawData.transform.GetWorldPosition(mesh.vertexPositionsBuffer.Load(vertexIndex)), 1.f));

        vertices[groupThreadId].position = TransformSVPosition(position);
        vertices[groupThreadId].objectId = drawData.entityId;
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