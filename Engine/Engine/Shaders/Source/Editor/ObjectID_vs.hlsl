#include "CommonBuffers.hlsli"
#include "DefaultVertexMeshlet.hlsli"
#include "Utility.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
};

Output main(in DefaultInput input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const float4 worldPosition = mul(input.GetTransform(), float4(input.GetVertexPositionData().position, 1.f));

    const GPUScene scene = constants.gpuScene.Load(0);
    const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(input.GetObjectID());

    Output output;
    output.position = mul(viewData.projection, mul(viewData.view, worldPosition)); // #TODO_Ivar: Switch to viewProjection
    output.objectId = drawData.entityId;
    return output;
}