#include "PushConstant.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Lights.hlsli"
#include "Structures.hlsli"

struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;
    UniformBuffer<DirectionalLight> directionalLight;
};

#define OVERRIDE_DEFAULT_CONSTANTS
#include "MeshShaderCommon.hlsli"

struct PerDrawData
{
    uint drawIndex;
    uint viewIndex;
};

PUSH_CONSTANT(PerDrawData, u_perDrawData);

struct VertexOutput
{
    float4 position : SV_Position;
};

struct PrimitiveOutput
{
    uint target : SV_RenderTargetArrayIndex;
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
    const DirectionalLight dirLight = constants.directionalLight.Load();

    const ObjectDrawData drawData = constants.gpuScene.objectDrawDataBuffer.Load(u_perDrawData.drawIndex);    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);
    const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + groupId);

    const uint vertexCount = meshlet.GetVertexCount();
    const uint triCount = meshlet.GetTriangleCount();

    SetMeshOutputCounts(vertexCount, triCount);

    if (groupThreadId < triCount)
    {
        const uint primitive = mesh.meshletDataBuffer.Load(meshlet.GetIndexOffset() + groupThreadId);
        tris[groupThreadId] = UnpackPrimitive(primitive);
        primitives[groupThreadId].target = u_perDrawData.viewIndex;
    }

    if (groupThreadId < vertexCount)
    {
        const uint vertexIndex = mesh.meshletDataBuffer[meshlet.GetVertexOffset() + groupThreadId] + mesh.vertexStartOffset;

        float4 position = mul(dirLight.viewProjections[u_perDrawData.viewIndex], float4(drawData.transform.GetWorldPosition(mesh.vertexPositionsBuffer.Load(vertexIndex)), 1.f));

        vertices[groupThreadId].position = TransformClipPosition(position);
    }
}

struct ColorOutput
{
    [[vt::d32f]];
};

struct VertexInput
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;
};

ColorOutput MainPS(VertexInput input)
{
    ColorOutput output;
    return output;
}