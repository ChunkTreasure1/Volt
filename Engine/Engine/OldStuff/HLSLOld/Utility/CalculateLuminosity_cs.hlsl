#include "Common.hlsli"
#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

#include "PBRHelpers.hlsli"

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);
Texture2D<float4> u_colorSource : register(t1, SPACE_OTHER);

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
    
    const float3 srcColor = u_colorSource[pixelCoords].rgb;
    o_resultTexture[pixelCoords] = CalculateLuminanceFromLight(srcColor);
}