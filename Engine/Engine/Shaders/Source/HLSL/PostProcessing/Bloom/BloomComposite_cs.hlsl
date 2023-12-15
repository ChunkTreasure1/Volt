#include "Common.hlsli"
#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);
Texture2D<float4> u_bloomSource : register(t1, SPACE_OTHER);

struct PushConstants
{
    float bloomStrength;
};

[[vk::push_constant]] PushConstants u_pushConstants;

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = RemapThreadIDToPixel(groupIndex, groupId.xy);

    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float bloomStrength = 0.04f * u_pushConstants.bloomStrength;
    
    const float3 srcColor = o_resultTexture[pixelCoords].rgb;
    const float3 bloomColor = u_bloomSource[pixelCoords].rgb;
    
    const float3 result = lerp(srcColor, bloomColor, bloomStrength);
    o_resultTexture[pixelCoords] = float4(result, 1.f);
}