#include "Buffers.hlsli"

struct Input
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;

    float3 worldPosition : WORLDPOSITION;
    uint currentLight : CURRENTLIGHT;
};

float main(Input input) : SV_Depth
{
    const PointLight pointLight = u_pointLights[input.currentLight];
    
    float lightDistance = length(input.worldPosition - pointLight.position.xyz);
    lightDistance = lightDistance / pointLight.radius;
    return lightDistance;
}