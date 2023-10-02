#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
};

struct PushConstants
{
    uint spotlightIndex;
};

[[vk::push_constant]] PushConstants u_pushConstants;

Output main(in DefaultVertexInput input)
{
    const ObjectData objectData = input.GetObjectData();
    float4x4 skinningMatrix = IDENTITY_MATRIX;
    
    if (objectData.isAnimated)
    {
        skinningMatrix = input.GetSkinnedMatrix();
    }
    
    const float4 worldPosition = mul(objectData.transform, mul(skinningMatrix, float4(input.position, 1.f)));
    const SpotLight spotlightData = u_spotLights[u_pushConstants.spotlightIndex];
    
    Output output = (Output) 0;
    output.position = mul(spotlightData.viewProjection, worldPosition);
    return output;
}