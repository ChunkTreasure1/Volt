#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

#ifndef POST_PROCESSING_BASE_H
#define POST_PROCESSING_BASE_H

#define POST_PROCESSING_THREAD_COUNT 8 

uint2 GetPixelCoords(uint groupIndex, uint3 groupId)
{
    return RemapThreadIDToPixel(groupIndex, groupId.xy);
}

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);

float2 GetUV(in uint2 pixelCoords)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);
    
    const float2 texCoords = float2((float) pixelCoords.x / (float) (outputWidth - 1), (float) pixelCoords.y / (float) (outputHeight - 1));
    return texCoords;
}

float4 GetPreviousColor(in uint2 pixelCoords)
{
    return o_resultTexture[pixelCoords];
}

void WriteColor(in float4 color, in uint2 pixelCoords)
{
    o_resultTexture[pixelCoords] = color;
}

#endif