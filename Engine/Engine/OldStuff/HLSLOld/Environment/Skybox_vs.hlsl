#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float3, samplePosition, SAMPLEPOSITION, 0);
    STAGE_VARIABLE(float3, worldPosition, WORLDPOSITION, 1);
};

Output main(in DefaultVertexInput input)
{
    Output output;

    const float3x3 view = (float3x3) u_cameraData.view;
    float4x4 rotView = IDENTITY_MATRIX;
    rotView[0].xyz = view[0];
    rotView[1].xyz = view[1];
    rotView[2].xyz = view[2];

    const float4 clipPos = mul(u_cameraData.projection, mul(rotView, float4(input.position, 1.0f)));

    const float3 dir = normalize(float3(u_cameraData.view[0][2], u_cameraData.view[1][2], u_cameraData.view[2][2]));
    
    output.position = clipPos;
    output.samplePosition = input.position;
    output.worldPosition = input.position * dir + u_cameraData.inverseView[3].xyz;

    return output;
}