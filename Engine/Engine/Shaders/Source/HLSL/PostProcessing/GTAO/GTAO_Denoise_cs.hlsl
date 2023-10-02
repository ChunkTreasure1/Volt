#include "Defines.hlsli"
#include "SamplerStates.hlsli"

#include "XeGTAO.hlsli"

Texture2D<uint> u_aoTerm : register(t0, SPACE_OTHER);
Texture2D<lpfloat> u_edges : register(t1, SPACE_OTHER);

RWTexture2D<uint> o_finalAOTerm : register(u2, SPACE_OTHER);

[[vk::push_constant]] GTAOConstants u_pushConstants;

[numthreads(8, 8, 1)]
void main(const uint2 dispatchThreadID : SV_DispatchThreadID)
{
    const uint2 pixCoordBase = dispatchThreadID * uint2(2, 1); // we're computing 2 horizontal pixels at a time (performance optimization)
    // g_samplerPointClamp is a sampler with D3D12_FILTER_MIN_MAG_MIP_POINT filter and D3D12_TEXTURE_ADDRESS_MODE_CLAMP addressing mode
    XeGTAO_Denoise(pixCoordBase, u_pushConstants, u_aoTerm, u_edges, u_pointSamplerClamp, o_finalAOTerm, false);
}