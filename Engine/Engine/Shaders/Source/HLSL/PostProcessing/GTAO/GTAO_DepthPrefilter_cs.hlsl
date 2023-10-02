#include "Defines.hlsli"
#include "SamplerStates.hlsli"

#include "XeGTAO.hlsli"

Texture2D<float> u_sourceDepth : register(t0, SPACE_OTHER);

RWTexture2D<lpfloat> u_outDepthMIP0 : register(u1, SPACE_OTHER);
RWTexture2D<lpfloat> u_outDepthMIP1 : register(u2, SPACE_OTHER);
RWTexture2D<lpfloat> u_outDepthMIP2 : register(u3, SPACE_OTHER);
RWTexture2D<lpfloat> u_outDepthMIP3 : register(u4, SPACE_OTHER);
RWTexture2D<lpfloat> u_outDepthMIP4 : register(u5, SPACE_OTHER);

[[vk::push_constant]] GTAOConstants u_pushConstants;

// Engine-specific entry point for the first pass
[numthreads(8, 8, 1)] // <- hard coded to 8x8; each thread computes 2x2 blocks so processing 16x16 block: Dispatch needs to be called with (width + 16-1) / 16, (height + 16-1) / 16
void main(uint2 dispatchThreadID : SV_DispatchThreadID, uint2 groupThreadID : SV_GroupThreadID)
{
    XeGTAO_PrefilterDepths16x16(dispatchThreadID, groupThreadID, u_pushConstants, u_sourceDepth, u_pointSamplerClamp, u_outDepthMIP0, u_outDepthMIP1, u_outDepthMIP2, u_outDepthMIP3, u_outDepthMIP4);
}