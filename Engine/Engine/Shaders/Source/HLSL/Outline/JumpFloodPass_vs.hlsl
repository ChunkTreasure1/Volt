#include "Vertex.hlsli"

struct Output
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    float2 texelSize : TEXELSIZE;
    float2 UV[9] : UV0;
};

struct PushConstants
{
    float2 texelSize;
    int step;
    int padding;
};

[[vk::push_constant]] PushConstants u_pushConstants;

Output main(const uint vertexIndex : SV_VertexID)
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
    
    Output output;
    output.texCoords = texCoords[vertexIndex];
    output.texelSize = u_pushConstants.texelSize;

    float2 dx = float2(u_pushConstants.texelSize.x, 0.f) * u_pushConstants.step;
    float2 dy = float2(0.f, u_pushConstants.texelSize.y) * u_pushConstants.step;

    output.UV[0] = output.texCoords;

	// Create sample positions in a 3x3 grid
    output.UV[1] = output.texCoords + dx;
    output.UV[2] = output.texCoords - dx;
    output.UV[3] = output.texCoords + dy;
    output.UV[4] = output.texCoords - dy;
    output.UV[5] = output.texCoords + dx + dy;
    output.UV[6] = output.texCoords + dx - dy;
    output.UV[7] = output.texCoords - dx + dy;
    output.UV[8] = output.texCoords - dx - dy;

    output.position = positions[vertexIndex];
    return output;
}