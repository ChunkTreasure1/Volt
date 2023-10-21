#include "Defines.hlsli"

Texture2D<uint2> u_visibilityBuffer : register(t0, space0);
StructuredBuffer<uint> u_materialCountBuffer : register(t1, space0);
StructuredBuffer<uint> u_materialStartBuffer : register(t2, space0);
StructuredBuffer<uint2> u_pixelCollection : register(t3, space0);

RWTexture2D<float4> o_albedo : register(u4, space0);
RWTexture2D<float4> o_materialEmissive : register(u5, space0);
RWTexture2D<float4> o_normalEmissive : register(u6, space0);

struct Data
{
    uint materialId;
};

PUSH_CONSTANT(Data, u_data);

groupshared uint m_materialCount;
groupshared uint m_materialStart;

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex)
{
    if (groupThreadIndex == 0)
    {
        m_materialCount = u_materialCountBuffer[u_data.materialId];
        m_materialStart = u_materialStartBuffer[u_data.materialId];
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    const uint pixelIndex = m_materialStart + threadId.x;
    
    if (pixelIndex >= m_materialCount)
    {
        return;
    }
    
    
    
    const uint2 pixelPosition = u_pixelCollection[pixelIndex];
    const uint2 visibilityValues = u_visibilityBuffer.Load(int3(pixelPosition, 0));

    o_albedo[pixelPosition] = 1.f;
}