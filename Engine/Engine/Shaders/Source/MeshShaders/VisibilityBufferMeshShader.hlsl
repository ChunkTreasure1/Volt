#include "PushConstant.hlsli"
#include "MeshShaderCommon.hlsli"

#include "MeshletHelpers.hlsli"

struct PerDrawData
{
    uint drawIndex;
};

PUSH_CONSTANT(PerDrawData, u_perDrawData);

struct VertexOutput
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
    uint meshletId : MESHLET_ID;
};

struct PrimitiveOutput
{
    uint primitiveID : SV_PrimitiveID;
};

[numthreads(NUM_MS_THREADS, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
            out indices uint3 tris[NUM_MAX_OUT_TRIS],
            out vertices VertexOutput vertices[NUM_MAX_OUT_VERTS],
            out primitives PrimitiveOutput primitives[NUM_MAX_OUT_TRIS])
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
        primitives[groupThreadId].primitiveID = groupThreadId;
    }

    if (groupThreadId < meshlet.vertexCount)
    { 
        const uint vertexIndex = mesh.meshletDataBuffer[vertexOffset + groupThreadId] + mesh.vertexStartOffset;
        const float4x4 transform = drawData.transform;

        const float3x3 worldNormalRotation = (float3x3)transform;
        const float3x3 cameraNormalRotation = (float3x3)viewData.view;

        float4 position = mul(viewData.viewProjection, mul(transform, float4(mesh.vertexPositionsBuffer.Load(vertexIndex), 1.f)));

        vertices[groupThreadId].position = TransformSVPosition(position);
        vertices[groupThreadId].objectId = u_perDrawData.drawIndex;
        vertices[groupThreadId].meshletId = groupId;
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