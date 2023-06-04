#include "Common.hlsli"
#include "SamplerStates.hlsli"
#include "Poisson.hlsli"

#ifndef SHADOWMAPPING_H
#define SHADOWMAPPING_H

float3 GetShadowMapCoords(in float4x4 viewProj, in float3 worldPosition)
{
    const float4 lightViewCoords = mul(viewProj, float4(worldPosition, 1.f));
    return lightViewCoords.xyz / lightViewCoords.w;
}

float GetDirectionalShadowBias(in DirectionalLight light, in uint cascadeIndex, in float3 normal)
{
    const float minShadowBias = 0.0005f / float(cascadeIndex + 1);
    const float bias = max(minShadowBias * (1.f - dot(normal, light.direction.xyz)), minShadowBias);
    return bias;
}

uint GetCascadeIndexFromWorldPosition(in DirectionalLight light, in float3 worldPosition, in float4x4 viewMatrix)
{
    const float4 viewSpacePosition = mul(viewMatrix, float4(worldPosition, 1.f));
    
    const int cascadeCount = 5;
    int cascadeIndex = 0;
    
    [unroll]
    for (int i = 0; i < cascadeCount; i++)
    {
        if (viewSpacePosition.z < light.cascadeDistances[i].x)
        {
            cascadeIndex = i;
            break;
        }
    }
  
    if (cascadeIndex == -1)
    {
        cascadeIndex = cascadeCount - 1;
    }
    
    return (uint) cascadeIndex;
}

float PCSS_SearchWidth(in float3 cameraPosition, float uvLightSize, float receiverDistance)
{
    const float near = 0.1f;
    return uvLightSize * (receiverDistance - near) / cameraPosition.z;
}

float PCSS_SearchRegionRadiusUV(float zWorld)
{
    const float lightZNear = 0.f;
    const float lightRadiusUV = 0.05f;
    
    return lightRadiusUV * (zWorld - lightZNear) / zWorld;
}

float PCSS_FindBlockerDistance(in DirectionalLight light, in Texture2DArray shadowMap, in float3 normal, in uint cascade, in float3 shadowCoords)
{
    const float bias = GetDirectionalShadowBias(light, cascade, normal);
    const int numBlockerSearchSamples = 64;
    const float searchWidth = PCSS_SearchRegionRadiusUV(shadowCoords.z);
    const float2 sampleCoords = float2(shadowCoords.x * 0.5f + 0.5f, -shadowCoords.y * 0.5f + 0.5f);
    
    int blockers = 0;
    float avgBlockerDistance = 0.f;
    
    for (int i = 0; i < numBlockerSearchSamples; i++)
    {
        const float z = shadowMap.SampleLevel(u_linearSampler, float3(sampleCoords + SamplePoisson((uint) i) * searchWidth, (float) cascade), 0).r;
        if (z < (shadowCoords.z - bias))
        {
            blockers++;
            avgBlockerDistance += z;
        }
    }
    
    if (blockers > 0)
    {
        return avgBlockerDistance / float(blockers);
    }
    
    return -1.f;
}

float PCF_DirectionalLight(in DirectionalLight light, in Texture2DArray shadowMap, in float3 normal, in uint cascadeIndex, in float3 shadowCoords, float uvRadius)
{
    const float bias = GetDirectionalShadowBias(light, cascadeIndex, normal);
    const int numPCFSamples = 64;
    
    const float2 sampleCoords = float2(shadowCoords.x * 0.5f + 0.5f, -shadowCoords.y * 0.5f + 0.5f);
    
    float result = 0.f;
    for (int i = 0; i < numPCFSamples; i++)
    {
        const float2 offset = SamplePoisson((uint) i) * uvRadius;
        const float z = shadowMap.SampleLevel(u_linearSampler, float3(sampleCoords + offset, (float) cascadeIndex), 0).r;
        result += step(shadowCoords.z - bias, z);
    }

    return result / numPCFSamples;
}

float PCF_NV_DirectionalLight(in DirectionalLight light, in Texture2DArray shadowMap, in float3 normal, in uint cascadeIndex, in float3 shadowCoords, float uvRadius)
{
    const float bias = GetDirectionalShadowBias(light, cascadeIndex, normal);
    const float2 sampleCoords = float2(shadowCoords.x * 0.5f + 0.5f, -shadowCoords.y * 0.5f + 0.5f);
    
    float result = 0.f;
    for (int i = 0; i < 16; i++)
    {
        const float2 offset = m_poissonDisk[i] * uvRadius;
        result += shadowMap.SampleCmpLevelZero(u_shadowSampler, float3(sampleCoords + offset, (float) cascadeIndex), shadowCoords.z - bias).r;
    }

    return result / 16.f;
}

float CalculateDirectionalShadow_Soft(in DirectionalLight light, in Texture2DArray shadowMap, in float3 normal, in uint cascadeIndex, in float3 shadowCoords)
{
    const float blockerDistance = PCSS_FindBlockerDistance(light, shadowMap, normal, cascadeIndex, shadowCoords);
    if (blockerDistance == -1.f)
    {
        return 1.f;
    }
    
    const float near = 0.01f;
    const float penumbraWidth = (shadowCoords.z - blockerDistance) / blockerDistance;
    float uvRadius = penumbraWidth * light.lightSize * near / shadowCoords.z;
    uvRadius = min(uvRadius, 0.002f);
    return PCF_NV_DirectionalLight(light, shadowMap, normal, cascadeIndex, shadowCoords, uvRadius);
}

float CalculateDirectionalShadow_Hard(in DirectionalLight light, in Texture2DArray shadowMap, in float3 normal, in uint cascadeIndex, in float3 shadowCoords)
{
    const float bias = GetDirectionalShadowBias(light, cascadeIndex, normal);
    const float2 sampleCoords = float2(shadowCoords.x * 0.5f + 0.5f, -shadowCoords.y * 0.5f + 0.5f);
    
    const float shadowMapDepth = shadowMap.SampleLevel(u_linearSampler, float3(sampleCoords, (float) cascadeIndex), 0.f).r;
    return step(shadowCoords.z, shadowMapDepth + bias);
}

///// Spot Lights /////
float CalculateSpotLightShadow(in SpotLight light, in Texture2D shadowMap, float3 worldPosition)
{
    const float3 shadowMapCoords = GetShadowMapCoords(light.viewProjection, worldPosition);
    const float2 sampleCoords = float2(shadowMapCoords.x * 0.5f + 0.5f, -shadowMapCoords.y * 0.5f + 0.5f);
    
    //if (any(shadowMapCoords.xyz < 0.f) || any(shadowMapCoords.xyz > 1.f))
    //{
    //    return 1.f;
    //}
    
    const float shadowMapDepth = shadowMap.SampleLevel(u_linearSampler, sampleCoords, 0.f).r;

    const float bias = 0.000025f;
    return step(shadowMapCoords.z, shadowMapDepth + bias);
}

///// Point Lights /////
float CalculatePointLightShadow(in PointLight light, in TextureCube shadowMap, float3 worldPosition)
{
    float3 fragToLight = worldPosition - light.position.xyz;
    float closestDepth = shadowMap.SampleLevel(u_linearSampler, fragToLight, 0.f).r;

    closestDepth *= light.radius;
    
    float currentDepth = length(fragToLight);
    
    float bias = 0.05f;
    return step(currentDepth, closestDepth + bias);
}
#endif