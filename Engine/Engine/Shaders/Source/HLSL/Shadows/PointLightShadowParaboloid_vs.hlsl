#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

float4 main(in DefaultVertexInput input) : POSITION
{
    const ObjectData objectData = input.GetObjectData();
    float4x4 skinningMatrix = IDENTITY_MATRIX;
    
    if (objectData.isAnimated)
    {
        skinningMatrix = input.GetSkinnedMatrix();
    }
    
    const float4 worldPosition = mul(objectData.transform, mul(skinningMatrix, float4(input.position, 1.f)));
    return worldPosition;
}