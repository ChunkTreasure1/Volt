#include "ComputeUtility.hlsli"
#include "SamplerStates.hlsli"

#include "Buffers.hlsli"
#include "PostProcessingBase.hlsli"
#include "Utility.hlsli"

Texture2D<float> u_depthTexture : register(t1, SPACE_OTHER);
Texture2D<float4> u_viewNormals : register(t2, SPACE_OTHER);
Texture2D<float4> u_sceneColor : register(t3, SPACE_OTHER);

static const float m_maxDistance = 1500.f;
static const float m_resolution = 0.3f;
static const int m_steps = 10;
static const float m_thickness = 50.f;

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = GetPixelCoords(groupIndex, groupId);
    
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float2 texSize = float2(outputWidth, outputHeight);
    
    const float2 texCoords = GetUV(pixelCoords);
    const float pixelDepth = u_depthTexture.Load(int3(pixelCoords, 0));
    
    float4 uv = 0.f;
    
    const float4 positionFrom = mul(u_cameraData.view, float4(ReconstructWorldPosition(texCoords, pixelDepth), 1.f));
    const float3 unitPositionFrom = normalize(mul((float3x3) u_cameraData.view, positionFrom.xyz));
    const float3 normal = normalize(u_viewNormals.Load(int3(pixelCoords, 0)).xyz);
    const float3 pivot = normalize(reflect(unitPositionFrom, normal));

    float4 positionTo = positionFrom;
    
    float4 startView = float4(positionFrom.xyz + (pivot * 0.f), 1.f);
    float4 endView = float4(positionFrom.xyz + (pivot * m_maxDistance), 1.f);
    
    float4 startFrag = startView;
    startFrag = mul(u_cameraData.projection, startFrag);
    startFrag.xyz /= startFrag.w;
    startFrag.xy = ClipToUV(startFrag.xy);
    startFrag.xy *= texSize;
    
    float4 endFrag = endView;
    endFrag = mul(u_cameraData.projection, endFrag);
    endFrag.xyz /= endFrag.w;
    endFrag.xy = ClipToUV(endFrag.xy);
    endFrag.xy *= texSize;
    
    float2 frag = startFrag.xy;
    uv.xy = frag / texSize;
    
    float deltaX = endFrag.x - startFrag.x;
    float deltaY = endFrag.y - startFrag.y;
    
    float useX = abs(deltaX) >= abs(deltaY) ? 1.f : 0.f;
    float delta = lerp(abs(deltaY), abs(deltaX), useX) * clamp(m_resolution, 0.f, 1.f);
    
    float2 increment = float2(deltaX, deltaY) / max(delta, 0.001f);
    
    float search0 = 0.f;
    float search1 = 0.f;
    
    int hit0 = 0;
    int hit1 = 0;
    
    float viewDistance = startView.z;
    float depth = m_thickness;
    
    for (int i = 0; i < int(delta); i++)
    {
        frag += increment;
        uv.xy = frag / texSize;
      
        const float currentDepth = u_depthTexture.SampleLevel(u_pointSampler, uv.xy, 0.f);
        positionTo = float4(ReconstructWorldPosition(uv.xy, pixelDepth), 1.f);
      
        search1 = lerp(((frag.y - startFrag.y) / deltaY), (frag.x - startFrag.x) / deltaX, useX);
        viewDistance = (startView.z * endView.z) / lerp(endView.z, startFrag.z, search1);

        depth = viewDistance - positionTo.z;
      
        if (depth > 0.f && depth < m_thickness)
        {
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
    }
    
    search1 = search0 + ((search1 - search0) / 2.f);

    int steps = m_steps * hit0;
    
    for (int i = 0; i < steps; i++)
    {
        frag = lerp(startFrag.xy, endFrag.xy, search1);
        uv.xy = frag / texSize;
      
        const float currentDepth = u_depthTexture.SampleLevel(u_pointSampler, uv.xy, 0.f);
        positionTo = float4(ReconstructWorldPosition(uv.xy, pixelDepth), 1.f);
    
        viewDistance = (startView.z * endView.z) / lerp(endView.z, startFrag.z, search1);
        depth = viewDistance - positionTo.z;
    
        if (depth > 0.f && depth < m_thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2.f);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2.f);
            search0 = temp;
        }
    }
    
    float visibility = hit1 * positionTo.w;
    visibility = visibility * (1.f - max(dot(-unitPositionFrom, pivot), 0.f));
    visibility = visibility * (1.f - clamp(depth / m_thickness, 0.f, 1.f));
    visibility = visibility * (1.f - clamp(length(positionTo - positionFrom) / m_maxDistance, 0.f, 1.f));
    visibility = visibility * (uv.x < 0.f || uv.x > 1.f ? 0.f : 1.f);
    visibility = visibility * (uv.y < 0.f || uv.y > 1.f ? 0.f : 1.f);

    visibility = clamp(visibility, 0.f, 1.f);
    uv.ba = visibility;
    
    const float4 reflectedColor = u_sceneColor.SampleLevel(u_pointSampler, uv.rg, 0.f);

    float alpha = clamp(visibility, 0, 1);
    const float4 finalColor = float4(lerp(0.f, reflectedColor.rgb, alpha), alpha);
    
    WriteColor(uv, pixelCoords);
}