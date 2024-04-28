#include "Resources.hlsli"
#include "Common.hlsli"
#include "Utility.hlsli"
#include "Lights.hlsli"

struct Constants
{
    UniformTexture<float> depthTexture;
    UniformBuffer<ViewData> viewData;
    
    UniformTypedBuffer<PointLight> pointLights;
    UniformTypedBuffer<SpotLight> spotLights;

    RWUniformTypedBuffer<int> visiblePointLightIndices;
    RWUniformTypedBuffer<int> visibleSpotLightIndices;
 
    uint2 tileCount;
};

groupshared uint m_minDepthInt;
groupshared uint m_maxDepthInt;
groupshared uint m_visiblePointLightCount;
groupshared uint m_visibleSpotLightCount;
groupshared float4 m_frustumPlanes[6];

groupshared uint m_visiblePointLights[MAX_LIGHTS_PER_TILE];
groupshared uint m_visibleSpotLights[MAX_LIGHTS_PER_TILE];

[numthreads(LIGHT_CULLING_TILE_SIZE, LIGHT_CULLING_TILE_SIZE, 1)]
void main(uint2 dispatchThreadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex, uint2 groupId : SV_GroupID)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const uint tileIndex = groupId.y * constants.tileCount.x + groupId.x;

    if (groupThreadIndex == 0)
    {
        m_minDepthInt = UINT32_MAX;
        m_maxDepthInt = 0;
        m_visiblePointLightCount = 0;
        m_visibleSpotLightCount = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    // Find max and min depth in current tile
    const float pixelDepthValue = LinearizeDepth(constants.depthTexture.Load2D(int3(dispatchThreadId, 0)), viewData);
    const uint depthInt = asuint(pixelDepthValue);
    
    InterlockedMin(m_minDepthInt, depthInt);
    InterlockedMax(m_maxDepthInt, depthInt);

    GroupMemoryBarrierWithGroupSync();

    // Calculate tile relative frustum planes
    if (groupThreadIndex == 0)
    {
        const float minDepth = asfloat(m_minDepthInt);
        const float maxDepth = asfloat(m_maxDepthInt);

        const float2 negativeStep = (2.f * (float2)groupId / (float2)constants.tileCount);
        const float2 positiveStep = (2.f * (float2)(groupId + 1.f) / (float2)constants.tileCount);

        m_frustumPlanes[0] = float4(1.f, 0.f, 0.f, 1.f - negativeStep.x); // Left
        m_frustumPlanes[1] = float4(-1.f, 0.f, 0.f, -1.f + positiveStep.x); // Right
        m_frustumPlanes[2] = float4(0.f, 1.f, 0.f, -1.f + negativeStep.y); // Bottom
        m_frustumPlanes[3] = float4(0.f, -1.f, 0.f, 1.f - positiveStep.y); // Top
        m_frustumPlanes[4] = float4(0.f, 0.f, 1.f, minDepth); // Near
        m_frustumPlanes[5] = float4(0.f, 0.f, -1.f, maxDepth); // Far
    
        [unroll]
        for (uint i = 0; i < 4; i++)
        {
            m_frustumPlanes[i] = mul(m_frustumPlanes[i], viewData.viewProjection);
            m_frustumPlanes[i] /= length(m_frustumPlanes[i].xyz);
        }

        m_frustumPlanes[4] = mul(m_frustumPlanes[4], viewData.view);
        m_frustumPlanes[4] /= length(m_frustumPlanes[4].xyz);

        m_frustumPlanes[5] = mul(m_frustumPlanes[5], viewData.view);
        m_frustumPlanes[5] /= length(m_frustumPlanes[5].xyz);
    }

    GroupMemoryBarrierWithGroupSync();

    // Cull lights

    const uint threadCount = LIGHT_CULLING_TILE_SIZE * LIGHT_CULLING_TILE_SIZE;
    uint passCount = DivideRoundUp(viewData.pointLightCount, threadCount);

    for (uint i = 0; i < passCount; i++)
    {
        uint lightIndex = i * threadCount + groupThreadIndex;
        if (lightIndex >= viewData.pointLightCount)
        {
            break;
        }

        const PointLight currentLight = constants.pointLights.Load(lightIndex);

        const float4 position = float4(currentLight.position, 1.f);
        const float radius = currentLight.radius * 1.2f; // We add some radius to remove some popping

        float distance = 0.f;

        [unroll]
        for (uint j = 0; j < 6; j++)
        {
            distance = dot(position, m_frustumPlanes[j]) + radius;
            if (distance <= 0.f)
            {
                // No intersection
                break;
            }        
        }

        if (distance > 0.f)
        {
            uint lightOffset;
            InterlockedAdd(m_visiblePointLightCount, 1, lightOffset);
            m_visiblePointLights[lightOffset] = lightIndex;
        }
    }

    passCount = DivideRoundUp(viewData.spotLightCount, threadCount);
    
    for (uint i = 0; i < passCount; i++)
    {
        uint lightIndex = i * threadCount + groupThreadIndex;
        if (lightIndex >= viewData.spotLightCount)
        {
            break;
        }

        const SpotLight light = constants.spotLights.Load(lightIndex);
        float distance = 0.f;
        
        [unroll]
        for (uint j = 0; j < 6; j++)
        {
            distance = dot(float4(light.position - light.direction * (light.range * 0.7f), 1.f), m_frustumPlanes[j]) + light.range * 1.3f;
            if (distance <= 0.f)
            {
                break;
            }
        }

        if (distance > 0.f)
        {
            uint lightOffset;
            InterlockedAdd(m_visibleSpotLightCount, 1, lightOffset);
            m_visibleSpotLights[lightOffset] = lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    // Put light indices into bins
    const uint offsetInBuffer = tileIndex * MAX_LIGHTS_PER_TILE;

    // Point Lights
    {
        const uint pointLightCount = m_visiblePointLightCount;

        for (uint i = groupThreadIndex; i < pointLightCount; i += threadCount)
        {
            constants.visiblePointLightIndices.Store(offsetInBuffer + i, m_visiblePointLights[i]);
        }

        if (groupThreadIndex == 0 && m_visiblePointLightCount != MAX_LIGHTS_PER_TILE)
        {
            constants.visiblePointLightIndices.Store(offsetInBuffer + pointLightCount, -1);
        }
    }

    // Spot Lights
    {
        const uint spotLightCount = m_visibleSpotLightCount;

        for (uint i = groupThreadIndex; i < spotLightCount; i += threadCount)
        {
            constants.visibleSpotLightIndices.Store(offsetInBuffer + i, m_visibleSpotLights[i]);
        }

        if (groupThreadIndex == 0 && m_visiblePointLightCount != MAX_LIGHTS_PER_TILE)
        {
            constants.visibleSpotLightIndices.Store(offsetInBuffer + spotLightCount, -1);
        }
    }
}