#include "Defines.hlsli"

Texture2D u_sssTexture : register(t0, SPACE_OTHER);
RWTexture2D<float4> u_outputTexture : register(u1, SPACE_OTHER);

[numthreads(16, 16, 1)]
void main(uint2 pixelCoords : SV_DispatchThreadID)
{
    const float3 diffuse = u_sssTexture.Load(int3(pixelCoords, 0)).rgb;
    u_outputTexture[pixelCoords].rgb += diffuse;
}