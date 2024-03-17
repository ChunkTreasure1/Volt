#include "Defines.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTypedBuffer<uint> dstBuffer;
    TypedBuffer<uint> srcBuffer;
    TypedBuffer<uint> scatterIndices;

    uint typeSizeInUINT;
    uint copyCount;
};

[numthreads(64, 1, 1)]
void main(uint dispatchThreadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();

    // Every thread copies one UINT
    uint scatterIndex = dispatchThreadId / constants.typeSizeInUINT;
    uint scatterOffset = dispatchThreadId - scatterIndex * constants.typeSizeInUINT;

    if (scatterIndex < constants.copyCount)
    {
        const uint dstIndex = constants.scatterIndices.Load(scatterIndex) * constants.typeSizeInUINT + scatterOffset;
        const uint srcIndex = dispatchThreadId;

        constants.dstBuffer.Store(dstIndex, constants.srcBuffer.Load(srcIndex));    
    }
}