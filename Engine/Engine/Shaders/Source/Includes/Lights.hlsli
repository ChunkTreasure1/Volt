#pragma once

// Culling
#define MAX_LIGHTS_PER_TILE 512
#define LIGHT_CULLING_TILE_SIZE 16

#define DIRECTIONAL_SHADOW_CASCADE_COUNT 4

struct DirectionalLight
{
    float4 direction;
    float3 color;
    float intensity;

    uint castShadows;

    float cascadeDistances[DIRECTIONAL_SHADOW_CASCADE_COUNT];
    float4x4 viewProjections[DIRECTIONAL_SHADOW_CASCADE_COUNT];
};

struct PointLight
{
    float3 position;
    float radius;
    
    float3 color;
    float intensity;
    
    float falloff;
    float3 padding;
};

struct SpotLight
{
    float3 position;
    float angleAttenuation;
    
    float3 color;
    float intensity;
    
    float3 direction;
    float range;
    
    float angle;
    float falloff;
    float2 padding;
};

int GetLightBufferIndex(vt::UniformTypedBuffer<int> lightIndexBuffer, uint tileCountX, int i, uint2 tileId)
{
    const uint index = tileId.y * tileCountX + tileId.x;
    const uint offset = index * MAX_LIGHTS_PER_TILE;

    return lightIndexBuffer.Load(offset + i);
}

int GetLightCount(vt::UniformTypedBuffer<int> lightIndexBuffer, uint tileCountX, uint2 tileId)
{
    int result = 0;
    for (int i = 0; i < MAX_LIGHTS_PER_TILE; i++)
    {
        int lightIndex = GetLightBufferIndex(lightIndexBuffer, tileCountX, i, tileId);
        if (lightIndex == -1)
        {
            break;
        }

        result++;
    }

    return result;
}

