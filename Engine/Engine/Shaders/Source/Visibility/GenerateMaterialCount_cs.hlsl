#include "Utility.hlsli"
#include "Common.hlsli"
#include "Structures.hlsli"

Texture2D<uint2> u_visibilityBuffer : register(t0, space0);
StructuredBuffer<IndirectDrawData> u_indirectDrawData : register(t1, space0);

RWStructuredBuffer<uint> u_materialCountsBuffer : register(u2, space0);

[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const uint2 pixelValue = u_visibilityBuffer.Load(int3(threadId.xy, 0));

    uint2 size;
    u_visibilityBuffer.GetDimensions(size.x, size.y);
    
    if (threadId.x >= size.x || threadId.y >= size.y)
    {
        return;
    }
    
    if (pixelValue.x == UINT32_MAX)
    {
        return;
    }
    
    IndirectDrawData objectData = u_indirectDrawData[pixelValue.x];
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
    
    InterlockedAdd(u_materialCountsBuffer[objectData.materialId], 1);
}