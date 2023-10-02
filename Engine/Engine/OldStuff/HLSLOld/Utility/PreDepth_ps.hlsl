#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"

struct Input
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float3, normal, NORMAL, 0);
    STAGE_VARIABLE(float2, texCoords, TEXCOORD, 1);
    STAGE_VARIABLE(uint, materialIndex, MATERIALINDEX, 2);
};

struct Output
{
    float4 viewNormals : SV_Target0;
};

Output main(Input input)
{
    const Material material = u_materialBuffer[input.materialIndex];
    float4 albedo = material.SampleAlbedo(u_linearSampler, input.texCoords);
    albedo.a = step(0.5f, albedo.a);
    
    if (albedo.a < 0.05f)
    {
        discard;
    }
    
    Output output = (Output) 0;
    output.viewNormals = float4(mul((float3x3) u_cameraData.view, input.normal), material.SampleMaterial(u_linearSampler, input.texCoords).g);
    return output;
}