#include "Resources.hlsli"
#include "ComputeUtilities.hlsli"

struct Constants
{
    RWUniformTypedBuffer<uint> indirectArgs;
    UniformTypedBuffer <uint> countBuffer;
  
    uint groupSize;
};

[numthreads(1, 1, 1)]
void main()
{
    const Constants constants = GetConstants<Constants>();
    
    uint count = constants.countBuffer.Load(0);
    const uint3 dispatchCount = GetGroupCountWrapped(count, constants.groupSize);
    
    constants.indirectArgs.Store(0, dispatchCount.x);
    constants.indirectArgs.Store(1, dispatchCount.y);
    constants.indirectArgs.Store(2, dispatchCount.z);
}