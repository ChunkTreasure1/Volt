#include "Vertex.hlsli"

struct PushConstants
{
    float4x4 viewProjectionTransform;
    float4 color;
    float2 vertexOffset;
    uint textureIndex;
};
[[vk::push_constant]] PushConstants u_pushConstants;

struct Output
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float4, color, COLOR, 0);
    STAGE_VARIABLE(float2, texCoords, TEXCOORDS, 1);
};

Output main(DefaultQuadVertex input)
{
    float4 tempPos = input.position;
    tempPos.xy += u_pushConstants.vertexOffset;
    
    Output output;
    output.position = mul(u_pushConstants.viewProjectionTransform, tempPos);
    output.texCoords = input.texCoords;
    output.color = u_pushConstants.color;

    return output;
}