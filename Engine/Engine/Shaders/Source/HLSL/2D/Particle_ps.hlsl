#include "SamplerStates.hlsli"
#include "Particle.hlsli"

#include "PBR.hlsli"

float4 main(in ParticleData input) : SV_Target
{
    //float4 albedo = input.SampleAlbedo(u_anisotropicSampler, input.texCoords) * input.color;
    //albedo.a = step(0.5f, albedo.a);
    
    //if (albedo.a < 0.05f)
    //{
    //    discard;
    //}
    
    //const float3 normal = float3(0.f, 1.f, 0.f);
    //const float4 rotatedNormal = mul(u_cameraData.inverseView, float4(normal, 0.f));
    
    //const float4 materialData = input.SampleMaterial(u_linearSampler, input.texCoords);
   
    //PBRData pbrData;
    //pbrData.albedo = albedo;
    //pbrData.normal = rotatedNormal.xyz;
    //pbrData.metallic = materialData.r;
    //pbrData.roughness = materialData.g;
    //pbrData.emissive = 0.f;
    //pbrData.worldPosition = input.worldPosition;
    //pbrData.tileId = input.position.xy / 16;
    
    //const float3 pbrLighting = CalculatePBR(pbrData);
    
    //return float4(pbrLighting, albedo.a);
    
    const float4 texColor = input.SampleAlbedo(u_anisotropicSampler, input.texCoords);
    if (texColor.a < 0.05f)
    {
        discard;
    }
    
    return texColor * input.color;
}