#include "Defines.hlsli"
#include "XeGTAO.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTexture<uint> finalAOTerm;
    TTexture<uint> aoTerm;
    TTexture<float> edges;
    TextureSampler pointClampSampler;
    
    GTAOConstants constants;
};

[numthreads(8, 8, 1)]
void main(const uint2 dispatchThreadID : SV_DispatchThreadID)
{
    const uint2 pixCoordBase = dispatchThreadID * uint2(2, 1); // we're computing 2 horizontal pixels at a time (performance optimization)
    const Constants constants = GetConstants<Constants>();
    
    Texture2D<uint> aoTerm = constants.aoTerm.Get2D();
    Texture2D<float> edges = constants.edges.Get2D();
    
    RWTexture2D<uint> finalAOTerm = constants.finalAOTerm.Get2D();
    SamplerState samplerState = constants.pointClampSampler.Get();
    
    // g_samplerPointClamp is a sampler with D3D12_FILTER_MIN_MAG_MIP_POINT filter and D3D12_TEXTURE_ADDRESS_MODE_CLAMP addressing mode
    XeGTAO_Denoise(pixCoordBase, constants.constants, aoTerm, edges, samplerState, finalAOTerm, false);
}