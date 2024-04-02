#include "Vertex.hlsli"

static const float4 m_positions[] =
{
    float4(-1.f, -1.f, 0.f, 1.f),
    float4(-1.f, 3.f, 0.f, 1.f),
    float4(3.f, -1.f, 0.f, 1.f),
};

static const float2 m_uvs[] =
{
    float2(0.f, 1.f),
    float2(0.f, -1.f),
    float2(2.f, 1.f)
};

FullscreenTriangleVertex main(const uint vertexIndex : SV_VertexID)
{
    FullscreenTriangleVertex output;
    output.position = m_positions[vertexIndex];
    output.uv = m_uvs[vertexIndex];

    return output;
}