#include "Bindless.hlsli"

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material
{
    uint albedoTexture;
    uint normalTexture;
    uint materialTexture;
    uint materialFlags;
    
    float4 color;
    float3 emissiveColor;
    float emissiveStrength;
    
    float roughness;
    float metalness;
    float normalStrength;
    
    float4 GetColor()
    {
        return color;
    }
    
    float3 GetEmissiveColor()
    {
        return emissiveColor;
    }
    
    float GetEmissiveStrength()
    {
        return emissiveStrength;
    }
    
    float GetRoughness()
    {
        return roughness;
    }
    
    float GetMetalness()
    {
        return metalness;
    }
    
    float GetNormalStrength()
    {
        return normalStrength;
    }
    
    uint GetFlags()
    {
        return materialFlags;
    }
    
    bool CastAO()
    {
        return (materialFlags & (1 << 5)) != 0;
    }
    
    // Build in derivatives
    float4 SampleAlbedoDerivatives(in SamplerState samplerState, in float2 texCoords, in float4 derivatives)
    {
        if (albedoTexture == 0)
        {
            return color;
        }
        
        return u_texture2DTable[NonUniformResourceIndex(albedoTexture)].SampleGrad(samplerState, texCoords, derivatives.xy, derivatives.zw) * color;
    }
    
    float4 SampleAlbedo(in SamplerState samplerState, in float2 texCoords)
    {
        if (albedoTexture == 0)
        {
            return color;
        }
        
        return u_texture2DTable[NonUniformResourceIndex(albedoTexture)].Sample(samplerState, texCoords) * color;
    }
    
    float3 SampleNormal(in SamplerState samplerState, in float2 texCoords, in float3x3 TBN)
    {
        float3 tangentNormal = float3(0.5f, 0.5f, 1.f);
        if (normalTexture != 0)
        {
            tangentNormal = u_texture2DTable[NonUniformResourceIndex(normalTexture)].Sample(samplerState, texCoords).wyz;
        }
        
        float3 resultNormal = tangentNormal * 2.f - 1.f;
        resultNormal.z = sqrt(1.f - saturate(resultNormal.x * resultNormal.x + resultNormal.y * resultNormal.y));
        resultNormal = lerp(resultNormal, float3(0.f, 1.f, 0.f), normalStrength);
        
        return normalize(mul(TBN, normalize(resultNormal)));
    }
    
    float3 SampleNormalDerivatives(in SamplerState samplerState, in float2 texCoords, in float4 derivatives, in float3x3 TBN)
    {
        float3 tangentNormal = float3(0.5f, 0.5f, 1.f);
        if (normalTexture != 0)
        {
            tangentNormal = u_texture2DTable[NonUniformResourceIndex(normalTexture)].SampleGrad(samplerState, texCoords, derivatives.xy, derivatives.zw).wyz;
        }
        
        float3 resultNormal = tangentNormal * 2.f - 1.f;
        resultNormal.z = sqrt(1.f - saturate(resultNormal.x * resultNormal.x + resultNormal.y * resultNormal.y));
        resultNormal = lerp(resultNormal, float3(0.f, 1.f, 0.f), normalStrength);
        
        return normalize(mul(TBN, normalize(resultNormal)));
    }
    
    float4 SampleMaterial(in SamplerState samplerState, in float2 texCoords)
    {
        if (materialTexture == 0)
        {
            return float4(metalness, roughness, 1.f, 0.f);
        }
        
        return u_texture2DTable[NonUniformResourceIndex(materialTexture)].Sample(samplerState, texCoords);
    }
    
    float4 SampleMaterialDerivatives(in SamplerState samplerState, in float2 texCoords, in float4 derivatives)
    {
        if (materialTexture == 0)
        {
            return float4(metalness, roughness, 1.f, 0.f);
        }
        
        return u_texture2DTable[NonUniformResourceIndex(materialTexture)].SampleGrad(samplerState, texCoords, derivatives.xy, derivatives.zw);
    }
};

StructuredBuffer<Material> u_materialBuffer : register(BINDING_MATERIALTABLE, SPACE_MAINBUFFERS);

Material GetMaterial(uint materialIndex)
{
    return u_materialBuffer[materialIndex];
}

float4 SampleTexture(in uint texture, in SamplerState samplerState, in float2 texCoords)
{
    if (texture == 0)
    {
        return 1.f;
    }
    
    return u_texture2DTable[NonUniformResourceIndex(texture)].Sample(samplerState, texCoords);
}

float4 SampleTextureLevel(in uint texture, SamplerState samplerState, in float2 texCoords, float mipLevel)
{
    if (texture == 0)
    {
        return 1.f;
    }
    
    return u_texture2DTable[NonUniformResourceIndex(texture)].SampleLevel(samplerState, texCoords, mipLevel);
}

uint2 GetTextureDimensions(in uint texture)
{
    if (texture == 0)
    {
        return 0;
    }
    
    uint2 result = 0;
    u_texture2DTable[NonUniformResourceIndex(texture)].GetDimensions(result.x, result.y);

    return result;
}

#endif