#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"
#include "Vertex.hlsli"

#include "PBR.hlsli"

struct Output
{
    float4 accumulation : SV_Target0;
    float revealage : SV_Target1;
};

Output main(DefaultPixelInput input)
{
    const float3x3 TBN = CalculateTBN(normalize(input.normal), normalize(input.tangent));
    
    const Material material = GetMaterial(input.materialIndex);
    
    const float4 albedo = material.SampleAlbedo(u_anisotropicSampler, input.texCoords);
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
    
    const float3 pbrLighting = CalculatePBR(pbrData);
    
    ///// Set these variables for output and it should work
    const float alpha = albedo.a;
    const float3 outColor = pbrLighting;
    /////
    
    float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 *
                         pow(1.0 - input.position.z * 0.9, 3.0), 1e-2, 3e3);
    
    Output output = (Output) 0;
    output.accumulation = float4(outColor * alpha, alpha) * weight;
    output.revealage = albedo.a;
    return output;
}