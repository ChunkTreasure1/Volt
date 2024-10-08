#include "Structures.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Common.hlsli"
#include "MeshletHelpers.hlsli"

#include "Atomics.hlsli"

struct Constants
{
    GPUScene gpuScene;

    vt::Tex2D<uint2> visibilityBuffer;
    vt::RWTypedBuffer<uint> materialCountsBuffer;

    uint2 renderSize;
};

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId.x >= constants.renderSize.x || threadId.y >= constants.renderSize.y)
    {
        return;
    }
    
    const uint2 pixelValue = constants.visibilityBuffer.Load(int3(threadId.xy, 0));
    
    if (pixelValue.x == UINT32_MAX)
    {
        return;
    }

    const PrimitiveDrawData objectData = constants.gpuScene.primitiveDrawDataBuffer.Load(pixelValue.x);
    if (objectData.materialId == UINT32_MAX)
    {
        return;
    }
     
    vt::InterlockedAdd(constants.materialCountsBuffer, objectData.materialId, 1);    
}