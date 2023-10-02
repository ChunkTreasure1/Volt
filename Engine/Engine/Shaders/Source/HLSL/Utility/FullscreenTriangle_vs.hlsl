#include "Vertex.hlsli"

DefaultFullscreenTriangleVertex main(const uint vertexIndex : SV_VertexID)
{
    const float4 positions[] =
    {
        float4(-1.f, -1.f, 0.f, 1.f),
        float4(-1.f, 3.f, 0.f, 1.f),
        float4(3.f, -1.f, 0.f, 1.f),
    };

    const float2 texCoords[3] =
    {
        float2(0.f, 1.f),
		float2(0.f, -1.f),
		float2(2.f, 1.f),
    };
    
    DefaultFullscreenTriangleVertex output;
    output.position = positions[vertexIndex];
    output.texCoords = texCoords[vertexIndex];
    
    return output;
}