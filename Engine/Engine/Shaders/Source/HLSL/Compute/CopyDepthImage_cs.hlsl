#include "Defines.hlsli"

Texture2D<float> u_sourceDepth : register(t0, SPACE_OTHER);
RWTexture2D<float4> o_result : register(u1, SPACE_OTHER);

[numthreads(16, 16, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixCoord = dispatchThreadID;
    
}