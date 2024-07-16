#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "PushConstant.hlsli"

struct Constants
{
    float4x4 viewProjection;

    UniformTypedBuffer<GPUMesh> gpuMeshesBuffer;
    UniformTypedBuffer<float4x4> transformsBuffer;
};

struct PerDrawData
{
    uint drawIndex;
};

PUSH_CONSTANT(PerDrawData, u_perDrawData);

struct VertexOutput
{
    float4 position : SV_Position;
};

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

[numthreads(64, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
            out indices uint3 tris[64],
            out vertices VertexOutput vertices[64])
{
    const Constants constants = GetConstants<Constants>();
    
    const GPUMesh mesh = constants.gpuMeshesBuffer.Load(u_perDrawData.drawIndex);
    const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + groupId);

    const float4x4 transform = constants.transformsBuffer.Load(u_perDrawData.drawIndex);

    const uint vertexCount = meshlet.GetVertexCount();
    const uint triCount = meshlet.GetTriangleCount();

    SetMeshOutputCounts(vertexCount, triCount);

    uint dataOffset = meshlet.dataOffset;
    uint vertexOffset = dataOffset;
    uint indexOffset = dataOffset + vertexCount;

    if (groupThreadId < triCount)
    {
        const uint primitive = mesh.meshletDataBuffer.Load(indexOffset + groupThreadId);
        tris[groupThreadId] = UnpackPrimitive(primitive);
    }

    if (groupThreadId < vertexCount)
    {
        const uint vertexIndex = mesh.meshletDataBuffer[vertexOffset + groupThreadId] + mesh.vertexStartOffset;
        vertices[groupThreadId].position = mul(constants.viewProjection, mul(constants.transformsBuffer.Load(u_perDrawData.drawIndex), float4(mesh.vertexPositionsBuffer.Load(vertexIndex), 1.f)));
    }
}

struct ColorOutput
{
	[[vt::rgba8]] float4 color : SV_Target0;
};

ColorOutput MainPS(VertexOutput input)
{
    ColorOutput output;
    output.color = float4(0.f, 1.f, 0.f, 1.f);
    return output;
}