#include "Defines.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTypedBuffer<uint> countsBuffer;
    uint size;
};

[numthreads(256, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{
    Constants constants = GetConstants<Constants>();

    if (threadId >= constants.size)
    {
        return;
    }
    
    
    constants.countsBuffer.Store(0, 0);
    constants.countsBuffer.Store(1, 0);
}