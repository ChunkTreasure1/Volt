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
    float4 worldPosition = mul(input.GetTransform(), float4(input.GetVertexPositionData().position, 1.f));
    
    const uint objectId = input.GetObjectID();
    
    Output output;
    output.position = mul(constants.cameraData.Load(0).projection, mul(constants.cameraData.Load(0).view, worldPosition));
    output.visId = input.GetPackedPrimitveID();
    
    return output;
}