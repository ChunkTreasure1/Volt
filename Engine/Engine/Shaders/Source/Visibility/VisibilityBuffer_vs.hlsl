#include "CommonBuffers.hlsli"
#include "DefaultVertexMeshlet.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint visId : VISID;
};

Output main(in DefaultInput input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();
    
    float4 worldPosition = mul(input.GetTransform(), float4(input.GetVertexPositionData().position, 1.f));
    
    const uint objectId = input.GetObjectID();
    
    Output output;
    output.position = mul(viewData.projection, mul(viewData.view, worldPosition)); // #TODO_Ivar: Switch to viewProjection
    output.visId = input.GetPackedPrimitveID();
    
    return output;
}