#include "CommonBuffers.hlsli"
#include "Utility.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Lights.hlsli"

struct Constants
{
    UniformTypedBuffer<GPUScene> gpuScene;
    UniformBuffer<ViewData> viewData;
    UniformTypedBuffer<DirectionalLight> directionalLight;
};

#define OVERRIDE_DEFAULT_CONSTANTS
#include "DefaultVertexMeshlet.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;
};

Output main(in DefaultInput input, in uint instanceId : SV_InstanceID)
{
    const Constants constants = GetConstants<Constants>();
    const DirectionalLight light = constants.directionalLight.Load(0);

    const float4x4 transform = input.GetTransform();

    Output output;
    output.target = instanceId;
    output.position = mul(light.viewProjections[instanceId], mul(transform, float4(input.GetVertexPositionData().position, 1.f)));

    return output;
}