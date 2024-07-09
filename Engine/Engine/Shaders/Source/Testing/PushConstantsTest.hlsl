#include "PushConstant.hlsli"

RWStructuredBuffer<uint> u_outputBuffer;

struct PushConstants
{
    uint value;
};

PUSH_CONSTANT(PushConstants, u_pushConstants);

[numthreads(1, 1, 1)]
void Main()
{
    u_outputBuffer[0] = u_pushConstants.value;
}