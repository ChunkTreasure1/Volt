#include "Vertex.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Material.hlsli"

struct Input
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float4, color, COLOR, 0);
    STAGE_VARIABLE(float2, texCoords, TEXCOORDS, 1);
};

struct PushConstants
{
    float4x4 viewProjectionTransform;
    float4 color;
    float2 vertexOffset;
    uint textureIndex;
};

[[vk::push_constant]] PushConstants u_pushConstants;

float4 main(Input input) : SV_Target
{
    const float4 texColor = SampleTexture(u_pushConstants.textureIndex, u_linearSamplerClamp, input.texCoords) * input.color;
    if (texColor.a < 0.05f)
    {
        discard;
    }
    
    return texColor;
}