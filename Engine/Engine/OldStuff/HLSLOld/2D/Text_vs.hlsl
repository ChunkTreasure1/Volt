#include <Vertex.hlsli>

struct Output
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoords : TEXCOORD;
    uint textureIndex : TEXINDEX;
};

struct PushConstants
{
    float4x4 viewProjection;
};
[[vk::push_constant]] PushConstants u_pushConstants;

Output main(DefaultTextVertex input)
{
    Output output;
    output.position = mul(u_pushConstants.viewProjection, input.position);
    output.texCoords = input.texCoords;
    output.color = input.color;
    output.textureIndex = input.textureIndex;

    return output;
}