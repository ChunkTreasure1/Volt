#include "CommonBuffers.hlsli"
#include "DefaultVertexMeshlet.hlsli"
#include "Utility.hlsli"

struct Output
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
};

Output main(in DefaultInput input)
{
    const Constants constants = GetConstants<Constants>();
    
    const float4x4 transform = input.GetTransform();
    const float4 worldPosition = mul(transform, float4(input.GetVertexPositionData().position, 1.f));
    //const float3 normal = mul((float3x3) transform, input.GetNormal());
    
    Output output;
    output.position = mul(constants.cameraData.Load(0).projection, mul(constants.cameraData.Load(0).view, worldPosition));
    output.normal = 1.f;
    return output;
}