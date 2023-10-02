#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"
#include "Vertex.hlsli"

struct Output
{
    float4 albedo : SV_Target0;
    float4 materialEmissive : SV_Target1; // Metallic, Roughness, Emissive R, Emissive G
    float4 normalEmissive : SV_Target2; // Normal, Emissive B
};

struct Input
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float3, tangent, TANGENT, 0);
    STAGE_VARIABLE(float3, normal, NORMAL, 1);
    STAGE_VARIABLE(half2, texCoords, TEXCOORD, 2);
    STAGE_VARIABLE(uint, materialIndex, MATINDEX, 3);
};

Output main(Input input)
{
    const float3x3 TBN = CalculateTBN(normalize(input.normal), normalize(input.tangent));
    
    Output output = (Output) 0;
    
    const Material material = GetMaterial(input.materialIndex);
    
    float4 albedo = material.SampleAlbedo(u_anisotropicSampler, input.texCoords);
    albedo.a = step(0.5f, albedo.a);
    
    if (albedo.a < 0.05f)
    {
        discard;
    }
    
    const float3 normal = material.SampleNormal(u_anisotropicSampler, input.texCoords, TBN);
    const float4 materialData = material.SampleMaterial(u_anisotropicSampler, input.texCoords);
    const float3 emissive = material.emissiveColor * material.emissiveStrength * materialData.b;
    
    output.albedo = albedo;
    output.materialEmissive = float4(materialData.r, materialData.g, emissive.rg);
    output.normalEmissive = float4(normal, emissive.b);
    
    return output;
}