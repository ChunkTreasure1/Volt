#pragma once

#include "Structures.hlsli"

static const float3 m_cascadeColors[DIRECTIONAL_SHADOW_CASCADE_COUNT] = 
{
    float3(1.f, 0.f, 0.f),
    float3(0.f, 1.f, 0.f),
    float3(0.f, 0.f, 1.f),
    float3(1.f, 1.f, 0.f),
    float3(1.f, 0.f, 1.f)
};

float3 GetCascadeColorFromIndex(uint cascadeIndex)
{
    return m_cascadeColors[cascadeIndex];
}

uint GetCascadeIndexFromWorldPosition(in DirectionalLight light, in float3 worldPosition, in float4x4 viewMatrix)
{
    const float4 viewSpacePosition = mul(viewMatrix, float4(worldPosition, 1.f));
    
    int cascadeIndex = -1;

    [unroll]
    for (int i = 0; i < DIRECTIONAL_SHADOW_CASCADE_COUNT; i++)
    {
        if (viewSpacePosition.z < light.cascadeDistances[i].x)
        {
            cascadeIndex = i;
            break;
        }
    }

    if (cascadeIndex == -1)
    {
        cascadeIndex = DIRECTIONAL_SHADOW_CASCADE_COUNT - 1;
    }

    return (uint)cascadeIndex;
}