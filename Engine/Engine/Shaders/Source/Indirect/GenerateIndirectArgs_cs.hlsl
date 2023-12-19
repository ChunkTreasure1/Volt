#include "Resources.hlsli"
#include "Utility.hlsli"

struct Constants
{
    RWTypedBuffer<uint> indirectArgs;
    TypedBuffer<uint> countBuffer;
    
    uint threadGroupSize;
};

[numthreads(1, 1, 1)]
void main()
{
    const Constants constants = GetConstants<Constants>();
    
    const uint count = constants.countBuffer.Load(0);
    constants.indirectArgs.Store(0, DivideRoundUp(count, constants.threadGroupSize));
    constants.indirectArgs.Store(1, 1);
    constants.indirectArgs.Store(2, 1);
}