#include "SamplerStates.hlsli"
#include "Utility.hlsli"
#include "Material.hlsli"

#include "ComputeUtility.hlsli"
#include "PBR.hlsli"
#include "Vertex.hlsli"

Texture2D<float4> u_albedoTexture : register(t0, SPACE_OTHER);
Texture2D<float4> u_materialEmissiveTexture : register(t1, SPACE_OTHER);
Texture2D<float4> u_normalEmissiveTexture : register(t2, SPACE_OTHER);
Texture2D<float4> u_depthTexture : register(t3, SPACE_OTHER);

RWTexture2D<float4> o_resultTexture : register(u4, SPACE_OTHER);

namespace RenderMode
{
    static const uint Default = 0;
    static const uint Normals = 1;
    static const uint Roughness = 2;
    static const uint Metallic = 3;
    static const uint WorldPosition = 4;
    static const uint LightComplexity = 5;
    static const uint Emissive = 6;
    static const uint AO = 7;
}

CONSTANT(uint, u_renderMode, 0, 0);

float3 CalculateWorldPosition(in uint2 pixelCoord, in float2 sampleCoords)
{
    const float depth = u_depthTexture.Load(int3(pixelCoord, 0)).r;
    return ReconstructWorldPosition(sampleCoords, depth);
}

float GetDepth(in float2 sampleCoords)
{
    return u_depthTexture.SampleLevel(u_pointSampler, sampleCoords, 0).x; 
}

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = RemapThreadIDToPixel(groupIndex, groupId.xy);
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float2 sampleCoords = float2(float(pixelCoords.x) / outputWidth, 1.f - (float(pixelCoords.y) / outputHeight));
    
    const float4 albedo = u_albedoTexture.Load(int3(pixelCoords, 0));
    
    if (albedo.a < 0.5f)
    {
        return;
    }
    
    float3 resultColor = 0.f;
    
    if (u_renderMode == RenderMode::Default)
    {
        const float4 materialEmissive = u_materialEmissiveTexture.Load(int3(pixelCoords, 0));
        const float4 normalEmissive = u_normalEmissiveTexture.Load(int3(pixelCoords, 0));
    
        PBRData pbrData;
        pbrData.albedo = albedo;
        pbrData.normal = normalEmissive.rgb;
        pbrData.metallic = materialEmissive.r;
        pbrData.roughness = materialEmissive.g;
        pbrData.emissive = float3(materialEmissive.b, materialEmissive.a, normalEmissive.a);
        pbrData.worldPosition = CalculateWorldPosition(pixelCoords, sampleCoords);
        pbrData.position = float4(pixelCoords, 0.f, 0.f);
        pbrData.castAO = true;
    
        resultColor = CalculatePBR(pbrData);
    }
    else if (u_renderMode == RenderMode::Normals)
    {
        const float4 normalEmissive = u_normalEmissiveTexture.Load(int3(pixelCoords, 0));
        resultColor = normalEmissive.xyz;
    }
    else if (u_renderMode == RenderMode::Roughness)
    {
        const float4 materialEmissive = u_materialEmissiveTexture.Load(int3(pixelCoords, 0));
        resultColor = materialEmissive.ggg;
    }
    else if (u_renderMode == RenderMode::Metallic)
    {
        const float4 materialEmissive = u_materialEmissiveTexture.Load(int3(pixelCoords, 0));
        resultColor = materialEmissive.rrr;
    }
    else if (u_renderMode == RenderMode::WorldPosition)
    {
        resultColor = CalculateWorldPosition(pixelCoords, sampleCoords);
    }
    else if (u_renderMode == RenderMode::LightComplexity)
    {
        const float4 materialEmissive = u_materialEmissiveTexture.Load(int3(pixelCoords, 0));
        const float4 normalEmissive = u_normalEmissiveTexture.Load(int3(pixelCoords, 0));
    
        PBRData pbrData;
        pbrData.albedo = albedo;
        pbrData.normal = normalEmissive.rgb;
        pbrData.metallic = materialEmissive.r;
        pbrData.roughness = materialEmissive.g;
        pbrData.emissive = float3(materialEmissive.b, materialEmissive.a, normalEmissive.a);
        pbrData.worldPosition = CalculateWorldPosition(pixelCoords, sampleCoords);
        pbrData.position = float4(pixelCoords, 0.f, 0.f);
        pbrData.castAO = true;
    
        resultColor = CalculatePBR(pbrData);
        
        int pointLightCount = GetPointLightCount(uint2(pbrData.position.xy / 16));
        float value = float(pointLightCount);
        resultColor.rgb = (resultColor.rgb * 0.2) + GetGradient(value);
    }
    else if (u_renderMode == RenderMode::Emissive)
    {
        const float4 materialEmissive = u_materialEmissiveTexture.Load(int3(pixelCoords, 0));
        const float4 normalEmissive = u_normalEmissiveTexture.Load(int3(pixelCoords, 0));
        resultColor = float3(materialEmissive.b, materialEmissive.a, normalEmissive.a);
    }
    else if (u_renderMode == RenderMode::AO)
    {
        float finalAO = CalculateAO(pixelCoords);
        resultColor = finalAO;
    }
    
    o_resultTexture[pixelCoords] = float4(resultColor, 1.f);
}