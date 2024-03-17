#include "Defines.hlsli"
#include "XeGTAO.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTexture<float> outDepthMIP0;
    RWTexture<float> outDepthMIP1;
    RWTexture<float> outDepthMIP2;
    RWTexture<float> outDepthMIP3;
    RWTexture<float> outDepthMIP4;
    
    TextureT<float> sourceDepth;
    TextureSampler pointClampSampler;
    uint padding;
    
    GTAOConstants constants;
};

// Engine-specific entry point for the first pass
[numthreads(8, 8, 1)] // <- hard coded to 8x8; each thread computes 2x2 blocks so processing 16x16 block: Dispatch needs to be called with (width + 16-1) / 16, (height + 16-1) / 16
void main(uint2 dispatchThreadID : SV_DispatchThreadID, uint2 groupThreadID : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    Texture2D<float> sourceDepth = constants.sourceDepth.Get2D();
    RWTexture2D<float> depthMip0 = constants.outDepthMIP0.Get2D();
    RWTexture2D<float> depthMip1 = constants.outDepthMIP1.Get2D();
    RWTexture2D<float> depthMip2 = constants.outDepthMIP2.Get2D();
    RWTexture2D<float> depthMip3 = constants.outDepthMIP3.Get2D();
    RWTexture2D<float> depthMip4 = constants.outDepthMIP4.Get2D();
    
    XeGTAO_PrefilterDepths16x16(dispatchThreadID, groupThreadID, constants.constants, sourceDepth, constants.pointClampSampler.Get(), depthMip0, depthMip1, depthMip2, depthMip3, depthMip4);
}