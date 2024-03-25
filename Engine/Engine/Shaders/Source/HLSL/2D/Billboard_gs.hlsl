#include "Vertex.hlsli"
#include "Buffers.hlsli"
#include "Particle.hlsli"

struct Output
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoords : UV;
    uint textureIndex : TEXTUREINDEX;
    uint id : ID;
};

[maxvertexcount(4)]
void main(point DefaultBillboardVertex input[1], inout TriangleStream<Output> output)
{
    const float2 offsets[4] =
    {
        { -100.f, 100.f },
        { 100.f, 100.f },
        { -100.f, -100.f },
        { 100.f, -100.f }
    };

    const float2 uvs[4] =
    {
        { 0.f, 0.f },
        { 1.f, 0.f },
        { 0.f, 1.f },
        { 1.f, 1.f }
    };
    
    const DefaultBillboardVertex inputData = input[0];
    for (uint i = 0; i < 4; i++)
    {
        Output result;
        result.position = mul(u_cameraData.view, inputData.position);
        result.position.xy += offsets[i] * inputData.scale.xy;
        result.position = mul(u_cameraData.projection, result.position);
        result.color = inputData.color;
        result.texCoords = uvs[i];
        result.textureIndex = inputData.textureIndex;
        result.id = inputData.id;

        output.Append(result);
    }
}