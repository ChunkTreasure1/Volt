#include "Bindless.hlsli"

#ifndef PARTICLE_H
#define PARTICLE_H

struct ParticleData
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION;
    float4 color : COLOR;
    float2 texCoords : UV;
    
    uint albedoIndex : ALBEDOINDEX;
    uint normalIndex : NORMALINDEX;
    uint materialIndex : MATERIALINDEX;
    
    float randomValue : RANDOMVALUE;
    float timeSinceSpawn : TIMESINCESPAWN;

    float4 SampleAlbedo(in SamplerState samplerState, in float2 texCoords)
    {
        if (albedoIndex == 0)
        {
            return 1.f;
        }
        
        return u_texture2DTable[NonUniformResourceIndex(albedoIndex)].Sample(samplerState, texCoords);
    }
    
    float3 SampleNormal(in SamplerState samplerState, in float2 texCoords)
    {
        if (normalIndex == 0)
        {
            return float3(0.f, 0.f, -1.f);
        }
        
        return u_texture2DTable[NonUniformResourceIndex(normalIndex)].Sample(samplerState, texCoords).xyz;
    }
    
    float4 SampleMaterial(in SamplerState samplerState, in float2 texCoords)
    {
        if (materialIndex == 0)
        {
            return float4(0.f, 1.f, 0.f, 0.f);
        }
        
        return u_texture2DTable[NonUniformResourceIndex(materialIndex)].Sample(samplerState, texCoords);
    }
};

#endif