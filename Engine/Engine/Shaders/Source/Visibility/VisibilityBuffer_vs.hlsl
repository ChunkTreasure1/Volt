#include "CommonBuffers.hlsli"
#include "DefaultVertexMeshlet.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint2 visId : VISID;
};

Output main(in DefaultInput input)
{
    const Constants constants = GetConstants<Constants>();
    float4 worldPosition = mul(input.GetTransform(), float4(input.GetVertexPositionData().position, 1.f));
    
    const uint triangleId = input.GetTriangleID();
    const uint objectId = input.GetObjectID();
    const uint meshletId = input.GetMeshletID();
    
    const uint triMeshletId = (triangleId << 16) | meshletId;
    
    Output output;
    output.position = mul(constants.cameraData.Load(0).projection, mul(constants.cameraData.Load(0).view, worldPosition));
    output.visId = uint2(objectId, triMeshletId);
    
    return output;
}