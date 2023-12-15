#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
};

struct PushConstants
{
    float4x4 transform;
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
    
    Output output = (Output) 0;
    output.position = mul(u_cameraData.projection, mul(u_cameraData.view, mul(u_pushConstants.transform, float4(input.position, 1.f))));
    return output;
}