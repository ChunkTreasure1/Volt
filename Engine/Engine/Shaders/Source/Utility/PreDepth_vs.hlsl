#include "CommonBuffers.hlsli"
#include "DefaultVertex.hlsli"
#include "Utility.hlsli"

struct Output
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
};

Output main(in DefaultInput input)
{
    const Constants constants = GetConstants<Constants>();
    const DrawContext context = constants.drawContext.Load(0);
    const GPUScene scene = constants.gpuScene.Load(0);
   
    const uint objectId = context.drawToInstanceOffset.Load(input.drawIndex);
    const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    
    const uint vertexIndex = mesh.indexBuffer.Load(input.vertexId) + mesh.vertexStartOffset;
    const VertexPositionData vertexPosition = mesh.vertexPositionsBuffer.Load(vertexIndex);
    
    float4 worldPosition = mul(drawData.transform, float4(vertexPosition.position, 1.f));
    
    Output output;
    output.position = mul(constants.cameraData.Load(0).projection, mul(constants.cameraData.Load(0).view, worldPosition));
    output.normal = float3(0.f, 1.f, 0.f);
    return output;
}