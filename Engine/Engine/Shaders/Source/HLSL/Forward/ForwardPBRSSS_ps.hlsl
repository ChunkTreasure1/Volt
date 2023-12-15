
#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"
#include "Vertex.hlsli"

#define SSS
#include "PBR.hlsli"

struct Output
{
    float4 diffuse : SV_Target0;
    float4 specular : SV_Target1;
};

Output main(DefaultPixelInput input)
{
    const float3x3 TBN = CalculateTBN(normalize(input.normal), normalize(input.tangent));
    
    const Material material = GetMaterial(input.materialIndex);
    
    float4 albedo = material.SampleAlbedo(u_anisotropicSampler, input.texCoords);
    albedo.a = step(0.5f, albedo.a);
    
    if (albedo.a < 0.05f)
    {
        discard;
    }
    
    const float3 normal = material.SampleNormal(u_linearSampler, input.texCoords, TBN);
    const float4 materialData = material.SampleMaterial(u_linearSampler, input.texCoords);
    const float3 emissive = material.emissiveColor * material.emissiveStrength;
    
    PBRData pbrData;
    pbrData.albedo = albedo;
    pbrData.normal = normal;
    pbrData.metallic = materialData.r;
    pbrData.roughness = materialData.g;
    pbrData.emissive = emissive * materialData.b;
    pbrData.worldPosition = input.worldPosition;
    pbrData.position = input.position;
    
    const LightingOutput pbrLighting = CalculatePBR(pbrData);
    
    Output output = (Output) 0;
    output.diffuse = float4(pbrLighting.diffuse, 1.f);
    output.specular = float4(pbrLighting.specular, 1.f);
    return output;
}