#include "Resources.hlsli"

#define GROUP_SIZE 32

struct Constants
{
    UniformRWTypedBuffer<uint> outputBuffer;
    uint initialValue;
};

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{ 
    const Constants constants = GetConstants<Constants>();
    constants.outputBuffer.Store(threadId, constants.initialValue + threadId);
} 