#include "Defines.hlsli"
#include "Resources.hlsli"

struct Constants
{
    TextureT<uint2> visibilityBuffer;
    TypedBuffer<uint> materialCountBuffer;
    TypedBuffer<uint> materialStartBuffer;
    TypedBuffer<uint2> pixelCollection;
    
    RWTexture<float4> albedo;
    RWTexture<float4> materialEmissive;
    RWTexture<float4> normalEmissive;
    
    uint materialId;
};

groupshared uint m_materialCount;
groupshared uint m_materialStart;

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex)
{
    const Constants constants = GetConstants<Constants>();
    
    if (groupThreadIndex == 0)
    {
        m_materialCount = constants.materialCountBuffer.Load(constants.materialId);
        m_materialStart = constants.materialStartBuffer.Load(constants.materialId);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    const uint pixelIndex = m_materialStart + threadId.x;
    
    if (pixelIndex >= m_materialCount)
    {
        return;
    }
    
    const uint2 pixelPosition = constants.pixelCollection.Load(pixelIndex);
    const uint2 visibilityValues = constants.visibilityBuffer.Load2D(int3(pixelPosition, 0));

    constants.albedo.Store2D(pixelPosition, 1.f);
}