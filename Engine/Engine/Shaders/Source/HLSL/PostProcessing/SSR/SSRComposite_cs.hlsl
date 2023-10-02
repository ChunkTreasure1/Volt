#include "ComputeUtility.hlsli"
#include "SamplerStates.hlsli"

#include "Buffers.hlsli"
#include "PostProcessingBase.hlsli"
#include "Utility.hlsli"

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
    
}