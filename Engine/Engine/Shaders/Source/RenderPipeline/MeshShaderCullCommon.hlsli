#pragma once

groupshared float3 m_vertexClipPositions[NUM_MS_THREADS];

void SetupCullingPositions(uint groupThreadId, float4 clipPosition, float2 renderSize)
{
    m_vertexClipPositions[groupThreadId] = float3((clipPosition.xy / clipPosition.w * 0.5f + 0.5f) * renderSize, clipPosition.w);
}

bool IsPrimitiveCulled(uint3 primitive)
{
    bool culled = false;
        
    float2 pa = m_vertexClipPositions[primitive.x].xy;
    float2 pb = m_vertexClipPositions[primitive.y].xy;
    float2 pc = m_vertexClipPositions[primitive.z].xy;

    // Backface and zero area culling.
    float2 eb = pb - pa;
    float2 ec = pc - pa;

    float2 bmin = min(pa, min(pb, pc));
    float2 bmax = max(pa, max(pb, pc));
    
    const float subPixelPrecision = 1.f / 256.f;
    
    culled = culled || (round(bmin.x - subPixelPrecision) == round(bmax.x) || round(bmin.y) == round(bmax.y + subPixelPrecision));
    culled = culled && (m_vertexClipPositions[primitive.x].z > 0.f && m_vertexClipPositions[primitive.y].z > 0.f && m_vertexClipPositions[primitive.z].z > 0.f); // Check that vertices are in front of perspective plane.

    return culled;
}