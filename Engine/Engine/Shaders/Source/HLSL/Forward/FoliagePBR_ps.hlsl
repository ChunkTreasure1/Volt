#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"
#include "Vertex.hlsli"

#include "PBR.hlsli"

struct Output
{
    float4 color : SV_Target0;
};

Output main(DefaultPixelInput input, bool isFrontFace : SV_IsFrontFace)
{
    const float3x3 TBN = CalculateTBN(normalize(input.normal), normalize(input.tangent));
    
    const Material material = GetMaterial(input.materialIndex);
    
    float4 albedo = material.SampleAlbedo(u_anisotropicSampler, input.texCoords);
    albedo.a = step(0.5f, albedo.a);
    
    if (albedo.a < 0.05f)
    {
        discard;
    }
    
    float3 normal = material.SampleNormal(u_linearSampler, input.texCoords, TBN);
    const float4 materialData = material.SampleMaterial(u_linearSampler, input.texCoords);
    const float3 emissive = material.emissiveColor * material.emissiveStrength;
    
    if (!isFrontFace)
    {
        normal *= -1.f;
    }
    
    PBRData pbrData;
    pbrData.albedo = albedo;
    pbrData.normal = normal;
    pbrData.metallic = materialData.r;
    pbrData.roughness = materialData.g;
    pbrData.emissive = emissive;
    pbrData.worldPosition = input.worldPosition;
    pbrData.position = input.position;
    pbrData.castAO = material.CastAO();
    
    const float3 pbrLighting = CalculatePBR(pbrData);
    
    Output output = (Output) 0;
    output.color = float4(pbrLighting, 1.f);
    return output;
}