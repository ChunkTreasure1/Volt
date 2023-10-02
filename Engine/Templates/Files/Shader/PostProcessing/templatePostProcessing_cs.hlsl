#include "Common.hlsli"
#include "Defines.hlsli"

#include "PostProcessingBase.hlsli"

[numthreads(POST_PROCESSING_THREAD_COUNT, POST_PROCESSING_THREAD_COUNT, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
    const uint2 pixelCoords = GetPixelCoords(groupIndex, groupId);
    
    ///// Safe Guard /////
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    //////////////////////
    
    const float4 srcColor = GetPreviousColor(pixelCoords);
    WriteColor(srcColor, pixelCoords);
}