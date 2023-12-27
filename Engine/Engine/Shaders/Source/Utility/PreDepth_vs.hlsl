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
    const ViewData viewData = constants.viewData.Load(0);
    
    const float4x4 transform = input.GetTransform();
    const float4 worldPosition = mul(transform, float4(input.GetVertexPositionData().position, 1.f));
    
    const float3x3 worldNormalRotation = (float3x3)transform;
    const float3x3 cameraNormalRotation = (float3x3)viewData.view;
    
    const float3 normal = mul(cameraNormalRotation, mul(worldNormalRotation, input.GetNormal()));
    
    Output output;
    output.position = mul(viewData.projection, mul(viewData.view, worldPosition)); // #TODO_Ivar: Switch to viewProjection
    output.normal = normal;
    return output;
}