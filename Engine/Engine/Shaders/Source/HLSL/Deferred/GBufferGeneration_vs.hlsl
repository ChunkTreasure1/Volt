#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float3, tangent, TANGENT, 0);
    STAGE_VARIABLE(float3, normal, NORMAL, 1);
    STAGE_VARIABLE(half2, texCoords, TEXCOORD, 2);
    STAGE_VARIABLE(uint, materialIndex, MATINDEX, 3);
};

Output main(in DefaultVertexInput input)
{
    const ObjectData objectData = input.GetObjectData();
    float4x4 skinningMatrix = IDENTITY_MATRIX;
    
    if (objectData.isAnimated)
    {
        skinningMatrix = input.GetSkinnedMatrix();
    }
    
    ///// TBN /////
    const float3x3 worldNormalRotation = (float3x3) objectData.transform;
    const float3x3 skinNormalRotation = (float3x3) skinningMatrix;
    
    const float3 T = normalize(mul(worldNormalRotation, mul(skinNormalRotation, input.GetTangent())));
    const float3 N = normalize(mul(worldNormalRotation, mul(skinNormalRotation, input.GetNormal())));
    
    Output output = (Output) 0;
    output.position = mul(u_cameraData.projection, mul(u_cameraData.view, mul(objectData.transform, mul(skinningMatrix, float4(input.position, 1.f)))));
    output.texCoords = input.texCoords;
    
    output.materialIndex = objectData.materialIndex;
    output.normal = N;
    output.tangent = T;
    
    return output;
}