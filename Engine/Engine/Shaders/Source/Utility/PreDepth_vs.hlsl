#include "CommonBuffers.hlsli"
#include "DefaultVertexMeshlet.hlsli"
#include "Utility.hlsli"

struct Output
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    
    uint vertexId : VERTEXID;
};

Output main(in DefaultInput input)
{
    const Constants constants = GetConstants<Constants>();
    const float4 worldPosition = mul(input.GetTransform(), float4(input.GetVertexPositionData().position, 1.f));
    
    Output output;
    output.position = mul(constants.cameraData.Load(0).projection, mul(constants.cameraData.Load(0).view, worldPosition));
    output.normal = float3(0.f, 1.f, 0.f);
    output.vertexId = input.vertexId;
    return output;
}