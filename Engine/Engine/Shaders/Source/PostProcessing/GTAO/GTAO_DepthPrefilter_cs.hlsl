#include "Defines.hlsli"
#include "XeGTAO.hlsli"
#include "Resources.hlsli"

struct Constants
{
    vt::RWTex2D<float> outDepthMIP0;
    vt::RWTex2D<float> outDepthMIP1;
    vt::RWTex2D<float> outDepthMIP2;
    vt::RWTex2D<float> outDepthMIP3;
    vt::RWTex2D<float> outDepthMIP4;
    
    vt::Tex2D<float> sourceDepth;
    vt::TextureSampler pointClampSampler;
    uint padding;
    
    GTAOConstants constants;
};

// Engine-specific entry point for the first pass
[numthreads(8, 8, 1)] // <- hard coded to 8x8; each thread computes 2x2 blocks so processing 16x16 block: Dispatch needs to be called with (width + 16-1) / 16, (height + 16-1) / 16
void main(uint2 dispatchThreadID : SV_DispatchThreadID, uint2 groupThreadID : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    Texture2D<float> sourceDepth = constants.sourceDepth.Get();
    RWTexture2D<float> depthMip0 = constants.outDepthMIP0.Get();
    RWTexture2D<float> depthMip1 = constants.outDepthMIP1.Get();
    RWTexture2D<float> depthMip2 = constants.outDepthMIP2.Get();
    RWTexture2D<float> depthMip3 = constants.outDepthMIP3.Get();
    RWTexture2D<float> depthMip4 = constants.outDepthMIP4.Get();
    
    XeGTAO_PrefilterDepths16x16(dispatchThreadID, groupThreadID, constants.constants, sourceDepth, constants.pointClampSampler.Get(), depthMip0, depthMip1, depthMip2, depthMip3, depthMip4);
}