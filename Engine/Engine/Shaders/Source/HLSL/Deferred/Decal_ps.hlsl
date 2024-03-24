#include "DecalCommon.hlsli"

struct Output
{
    float4 albedo : SV_Target0;
    float4 materialEmissive : SV_Target1; // Metallic, Roughness, Emissive R, Emissive G
    float4 normalEmissive : SV_Target2; // Normal, Emissive B
};

Output main(DecalOutput input)
{
    const float4x4 invTransform = inverse(u_materialData.transform);
    const float2 screenPos = input.position.xy / u_materialData.targetSize;
    
    const float depth = u_depthTexture.SampleLevel(u_pointSampler, screenPos, 0).r;
    const float3 pixelWorldPosition = ReconstructWorldPosition(float2(screenPos.x, 1.f - screenPos.y), depth);
    const float4 objectSpacePosition = mul(invTransform, float4(pixelWorldPosition, 1.f));
    
    clip(50.f - abs(objectSpacePosition.xyz));
    
    const float4 rotVec = normalize(mul(u_materialData.transform, float4(0.f, 0.f, 1.f, 0.f)));
    const float3 pixelNormal = normalize(cross(ddy(pixelWorldPosition), ddx(pixelWorldPosition)));
    
    float decalCull = saturate(dot(rotVec.xyz, pixelNormal));
    const float2 sampleCoords = -objectSpacePosition.xy + 0.5f;
    
    const Material material = GetMaterial(u_materialData.materialIndex);
    
    float4 albedo = material.SampleAlbedo(u_anisotropicSampler, sampleCoords);
    albedo *= decalCull;
    albedo.a = step(0.5f, albedo.a);
    
    if (albedo.a < 0.05f)
    {
        discard;
    }
    
    const float3 viewVector = normalize(u_cameraData.position.xyz - pixelWorldPosition.xyz);
    const float3x3 TBN = CalculateDecalTBN(pixelWorldPosition, viewVector, sampleCoords);
    
    ///// DO STUFF BELOW /////
    
    const float3 normal = material.SampleNormal(u_linearSampler, sampleCoords, TBN);
    const float4 materialData = material.SampleMaterial(u_linearSampler, sampleCoords);
    const float3 emissive = material.emissiveColor * material.emissiveStrength * materialData.b;

    Output output = (Output) 0;
    output.albedo = albedo;
    output.materialEmissive = float4(materialData.r, materialData.g, emissive.rg);
    output.normalEmissive = float4(normal, emissive.b);
    
    return output;
}