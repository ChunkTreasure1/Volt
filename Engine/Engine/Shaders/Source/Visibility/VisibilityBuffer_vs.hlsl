#include "CommonBuffers.hlsli"
#include "DefaultVertex.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint2 visId : VISID;
};

Output main(in DefaultInput input)
{
    float4 worldPosition = input.GetWorldPosition();
    
    const uint triangleId = input.GetTriangleID();
    const uint objectId = input.GetObjectID();
    
    Output output;
    output.position = mul(u_cameraData.projection, mul(u_cameraData.view, worldPosition));
    output.visId = uint2(objectId, triangleId);
    
    return output;
}