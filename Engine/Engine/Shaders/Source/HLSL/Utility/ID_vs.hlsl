#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
    STAGE_VARIABLE(uint, objectID, OBJECTID, 3);
};

Output main(in DefaultVertexInput input)
{
    const ObjectData objectData = input.GetObjectData();
    float4x4 skinningMatrix = IDENTITY_MATRIX;
    
    if (objectData.isAnimated)
    {
        skinningMatrix = input.GetSkinnedMatrix();
    }
    
    Output output = (Output) 0;
    output.position = mul(u_cameraData.projection, mul(u_cameraData.view, mul(objectData.transform, mul(skinningMatrix, float4(input.position, 1.f)))));
    output.objectID = objectData.id;
    
    return output;
}