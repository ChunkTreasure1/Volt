#include "Common.hlsli"
#include "Defines.hlsli"
#include "SamplerStates.hlsli"
#include "Utility.hlsli"

struct LightCullData
{
    uint2 numWorkGroups;
    uint2 targetSize;
};

[[vk::push_constant]] LightCullData u_cullData;

Texture2D u_srcDepth : register(t0, SPACE_OTHER);

groupshared uint m_minDepthInt;
groupshared uint m_maxDepthInt;
groupshared uint m_visibleLightCount;
groupshared float4 m_frustumPlanes[6];

groupshared uint m_visibleLightIndices[1024];
groupshared float4x4 m_viewProjection;

[numthreads(16, 16, 1)]
void main(uint2 location : SV_DispatchThreadID, uint itemId : SV_GroupIndex, uint2 tileId : SV_GroupID)
{
    uint index = tileId.y * u_cullData.numWorkGroups.x + tileId.x;
    
    if (itemId == 0)
    {
        m_minDepthInt = 0xFFFFFFFF;
        m_maxDepthInt = 0;

        m_visibleLightCount = 0;
        m_viewProjection = mul(u_cameraData.nonReversedProj, u_cameraData.view);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // Calculate minimum and maximum depth values
    float maxDepth, minDepth;
    float2 texCoords = float2(location) / u_cullData.targetSize;
    
    float depth = u_srcDepth.SampleLevel(u_pointSampler, texCoords, 0).x;
    depth = LinearizeDepth(depth);
    
    // Convert depth to uint to be able to perform atomic min and max comparisons
    uint depthInt = asuint(depth);
    InterlockedMin(m_minDepthInt, depthInt);
    InterlockedMax(m_maxDepthInt, depthInt);

    GroupMemoryBarrierWithGroupSync();
    
    // One thread calculates the frustum planes to be used for this tile
    if (itemId == 0)
    {
        minDepth = asfloat(m_minDepthInt);
        maxDepth = asfloat(m_maxDepthInt);
        
        float2 negativeStep = (2.f * float2(tileId)) / float2(u_cullData.numWorkGroups);
        float2 positiveStep = (2.f * float2(tileId + 1)) / float2(u_cullData.numWorkGroups);

        m_frustumPlanes[0] = float4(1.f, 0.f, 0.f, 1.f - negativeStep.x); // Left
        m_frustumPlanes[1] = float4(-1.f, 0.f, 0.f, -1.f + positiveStep.x); // Right
        m_frustumPlanes[2] = float4(0.f, 1.f, 0.f, 1.f - negativeStep.y); // Bottom
        m_frustumPlanes[3] = float4(0.f, -1.f, 0.f, -1.f + positiveStep.y); // Top
        m_frustumPlanes[4] = float4(0.f, 0.f, -1.f, -minDepth); // Near
        m_frustumPlanes[5] = float4(0.f, 0.f, 1.f, maxDepth); // Far
        
        // Transform the first four planes
        for (uint i = 0; i < 4; i++)
        {
            m_frustumPlanes[i] = mul(m_frustumPlanes[i], m_viewProjection);
            m_frustumPlanes[i] /= length(m_frustumPlanes[i].xyz);
        }

        // Transform the depth planes
        m_frustumPlanes[4] = mul(m_frustumPlanes[4], u_cameraData.view);
        m_frustumPlanes[4] /= length(m_frustumPlanes[4].xyz);
        m_frustumPlanes[5] = mul(m_frustumPlanes[5], u_cameraData.view);
        m_frustumPlanes[5] /= length(m_frustumPlanes[5].xyz);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // Perform light culling by seperating it out on 256 threads
    uint threadCount = 16 * 16;
    uint passCount = (u_sceneData.pointLightCount + threadCount - 1) / threadCount;
    
    for (uint i = 0; i < passCount; i++)
    {
        uint lightIndex = i * threadCount + itemId;
        if (lightIndex >= u_sceneData.pointLightCount)
        {
            break;
        }
        
        const PointLight light = u_pointLights[lightIndex];
        
        float4 position = light.position;
        float radius = light.radius;
        
        float dist = 0.f;
        for (uint j = 0; j < 6; j++)
        {
            dist = dot(position, m_frustumPlanes[j]) + radius;
            
            if (dist <= 0.f)
            {
                break;
            }
        }
        
        if (dist > 0.f)
        {
            uint offset; 
            InterlockedAdd(m_visibleLightCount, 1, offset);
            m_visibleLightIndices[offset] = (int) lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    
    if (itemId == 0)
    {
        uint offset = index * 1024;
        for (uint i = 0; i < m_visibleLightCount; i++)
        {
            u_visibleLightIndices[offset + i] = m_visibleLightIndices[i];
        }

        if (m_visibleLightCount != 1024)
        {
            u_visibleLightIndices[offset + m_visibleLightCount] = -1;
        }
    }
}
