#include "Defines.hlsli"
#include "SamplerStates.hlsli"
#include "PostProcessingBase.hlsli"

Texture2D<float> u_depthTexture : register(t1, SPACE_OTHER);
Texture2D<float4> u_colorImage : register(t2, SPACE_OTHER);
Texture2D<float4> u_historyImage : register(t3, SPACE_OTHER);

struct PushConstants
{
    float4x4 reprojectionMatrix;
    float2 texelSize;
};

[[vk::push_constant]] PushConstants u_pushConstants;

groupshared float4 m_ldsNeighbourHood[POST_PROCESSING_THREAD_COUNT][POST_PROCESSING_THREAD_COUNT];

float3 SampleHistory(in float2 texCoords, float4 rtMetrics)
{
    const float sharpening = 0.5f;
    
    float2 samplePos = texCoords * rtMetrics.xy;
    float2 tc1 = floor(samplePos - 0.5f) + 0.5f;
    float2 f = samplePos - tc1;
    float2 f2 = f * f;
    float2 f3 = f * f2;
    
    // Catmull-Rom weights
    const float c = sharpening;
    float2 w0 = -(c) * f3 + (2.0 * c) * f2 - (c * f);
    float2 w1 = (2.0 - c) * f3 - (3.0 - c) * f2 + 1.0;
    float2 w2 = -(2.0 - c) * f3 + (3.0 - 2.0 * c) * f2 + (c * f);
    float2 w3 = (c) * f3 - (c) * f2;

    float2 w12 = w1 + w2;
    float2 tc0 = (tc1 - 1.f) * rtMetrics.zw;
    float2 tc3 = (tc1 + 2.f) * rtMetrics.zw;
    float2 tc12 = (tc1 + w2 / w12) * rtMetrics.zw;

    // Bicubic filter using bilinear lookups, skipping the 4 corner texels
    float4 filtered = float4(u_historyImage.SampleLevel(u_linearSampler, float2(tc12.x, tc0.y), 0.f).rgb, 1.0) * (w12.x * w0.y) +
	                  float4(u_historyImage.SampleLevel(u_linearSampler, float2(tc0.x, tc12.y), 0.f).rgb, 1.0) * (w0.x * w12.y) +
	                  float4(u_historyImage.SampleLevel(u_linearSampler, float2(tc12.x, tc12.y), 0.f).rgb, 1.0) * (w12.x * w12.y) + // Center pixel
	                  float4(u_historyImage.SampleLevel(u_linearSampler, float2(tc3.x, tc12.y), 0.f).rgb, 1.0) * (w3.x * w12.y) +
	                  float4(u_historyImage.SampleLevel(u_linearSampler, float2(tc12.x, tc3.y), 0.f).rgb, 1.0) * (w12.x * w3.y);
    
    return filtered.rgb * (1.f / filtered.a);
}

float3 ClipAABB(float3 p, float3 aabbMin, float3 aabbMax)
{
    float3 center = 0.5f * (aabbMax + aabbMin);
    float3 halfSize = 0.5f * (aabbMax - aabbMin) * 1e-5;
    
    float3 clip = p - center;
    
    float3 unit = clip / halfSize;
    float3 absUnit = abs(unit);
    
    float maxUnit = max(absUnit.x, max(absUnit.y, absUnit.z));

    return (maxUnit > 1.f) ? clip * (1.f / maxUnit) + center : p;
}

[numthreads(POST_PROCESSING_THREAD_COUNT, POST_PROCESSING_THREAD_COUNT, 1)]
void main(uint3 inDispatchIdx : SV_DispatchThreadID, uint2 groupThreadId : SV_GroupThreadID)
{
    uint2 pixCoord = inDispatchIdx.xy;
 
    const float depth = u_depthTexture.Load(int3(pixCoord, 0)).r;
    const float2 texCoord = u_pushConstants.texelSize * float2(pixCoord) + 0.5f;
    const float4 reprojectedPos = mul(u_pushConstants.reprojectionMatrix, float4(texCoord * 2.f - 1.f, depth, 1.f));
    const float2 previousTexCoords = (reprojectedPos.xy / reprojectedPos.w) * 0.5f + 0.5f;
    
    const float3 currentColor = u_colorImage.Load(int3(pixCoord, 0)).rgb;
    
    m_ldsNeighbourHood[groupThreadId.x][groupThreadId.y] = float4(currentColor, 1.f);
    GroupMemoryBarrierWithGroupSync();
    
    float3 neighbourPlusMin = currentColor;
    float3 neighbourPlusMax = currentColor;
    
    float3 neighbourBoxMin = currentColor;
    float3 neighbourBoxMax = currentColor;
    
    const uint2 localId = groupThreadId;
    
    const int2 start = max(localId - 1, 0);
    const int2 end = min(localId + 2, int2(POST_PROCESSING_THREAD_COUNT, POST_PROCESSING_THREAD_COUNT));
    
    for (int y = start.y; y < end.y; y++)
    {
        for (int x = start.x; x < end.x; x++)
        {
            float3 color = m_ldsNeighbourHood[x][y].rgb;
            neighbourBoxMin = min(neighbourBoxMin, color);
            neighbourBoxMax = max(neighbourBoxMax, color);
            
            neighbourPlusMin = ((x == localId.x || y == localId.y) ? min(neighbourPlusMin, color) : neighbourPlusMin);
            neighbourPlusMax = ((x == localId.x || y == localId.y) ? max(neighbourPlusMax, color) : neighbourPlusMax);
        }
    }
    
    float3 neighbourMin = lerp(neighbourPlusMin, neighbourBoxMin, 0.5f);
    float3 neighbourMax = lerp(neighbourPlusMax, neighbourBoxMax, 0.5f);
    float3 halfSize = 0.5f * (neighbourMax - neighbourMin) * 1e-5;
    neighbourMin -= halfSize * 4.f;
    neighbourMax += halfSize * 4.f;
    
    float2 historySize;
    u_historyImage.GetDimensions(historySize.x, historySize.y);
    
    float3 historyColor = max(SampleHistory(previousTexCoords, float4(historySize, u_pushConstants.texelSize)), 0.f);
    historyColor = ClipAABB(historyColor, neighbourMin, neighbourMax);
    
    float alpha = 0.875f;
    alpha *= (any(previousTexCoords < 0.f) || any(previousTexCoords > 1.f)) ? 0.f : 1.f;
    
    float3 result = lerp(currentColor, historyColor, alpha);
    
    WriteColor(float4(result, 1.f), pixCoord);
}