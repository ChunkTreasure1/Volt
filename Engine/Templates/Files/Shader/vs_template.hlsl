#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

DefaultPixelInput main(in DefaultVertexInput input)
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
    
    DefaultPixelInput output = (DefaultPixelInput) 0;
    
    output.worldPosition = mul(objectData.transform, mul(skinningMatrix, float4(input.position, 1.f))).xyz;
    output.position = mul(u_cameraData.projection, mul(u_cameraData.view, float4(output.worldPosition, 1.f)));
    output.texCoords = input.texCoords;
    
    output.materialIndex = objectData.materialIndex;
    output.normal = N;
    output.tangent = T;
    output.paintedColor = input.GetPaintedColor();
    output.objectIndex = input.GetObjectIndex();
    
    return output;
}