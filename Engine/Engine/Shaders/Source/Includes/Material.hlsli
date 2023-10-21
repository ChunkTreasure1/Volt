#pragma once

#include "Resources.hlsli"
#include "Bindless.hlsli"

struct Material
{
    ResourceHandle textures[16];
    
    uint textureCount;
    uint materialFlags;
    uint2 padding;

    template<typename T>
    T SampleTexture(SamplerState samplerState, float2 texCoords, uint materialTextureIndex)
    {
        return (T)u_bindlessTexture2D[textures[materialTextureIndex].handle];
    }
    
    template<typename T>
    T LoadTexture(uint2 pixelPosition, uint materialTextureIndex)
    {
        return (T)u_bindlessTexture2D[textures[materialTextureIndex].handle];
    }
};

StructuredBuffer<Material> u_materials : register(t3, space0);