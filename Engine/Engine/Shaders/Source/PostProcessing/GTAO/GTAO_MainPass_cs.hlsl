#include "Defines.hlsli"
#include "XeGTAO.hlsli"
#include "Resources.hlsli"

struct Constants
{
    RWTexture<uint> aoTerm;
    RWTexture<float> edges;
    
    TTexture<float> srcDepth;
    TTexture<float4> viewspaceNormals;
    TextureSampler pointClampSampler;
    uint3 padding;
    
    GTAOConstants constants;
};

// Engine-specific screen & temporal noise loader
lpfloat2 SpatioTemporalNoise(uint2 pixCoord, uint temporalIndex)    // without TAA, temporalIndex is always 0
{
    float2 noise;
#if 1   // Hilbert curve driving R2 (see https://www.shadertoy.com/view/3tB3z3)
#ifdef XE_GTAO_HILBERT_LUT_AVAILABLE // load from lookup texture...
        uint index = g_srcHilbertLUT.Load( uint3( pixCoord % 64, 0 ) ).x;
#else // ...or generate in-place?
    uint index = HilbertIndex(pixCoord.x, pixCoord.y);
#endif
    index += 288 * (temporalIndex % 64); // why 288? tried out a few and that's the best so far (with XE_HILBERT_LEVEL 6U) - but there's probably better :)
    // R2 sequence - see http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
    return lpfloat2(frac(0.5 + index * float2(0.75487766624669276005, 0.5698402909980532659114)));
#else   // Pseudo-random (fastest but looks bad - not a good choice)
    uint baseHash = Hash32( pixCoord.x + (pixCoord.y << 15) );
    baseHash = Hash32Combine( baseHash, temporalIndex );
    return lpfloat2( Hash32ToFloat( baseHash ), Hash32ToFloat( Hash32( baseHash ) ) );
#endif
}

// Engine-specific normal map loader
lpfloat3 LoadNormal(int2 pos, Texture2D<float4> viewspaceNormals)
{
    float4 viewNormals = viewspaceNormals.Load(int3(pos, 0));
    return (lpfloat3) viewNormals.xyz;
}

[numthreads(16, 16, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    RWTexture2D<uint> aoTerm = constants.aoTerm.Get2D();
    RWTexture2D<float> edges = constants.edges.Get2D();
    
    Texture2D<float> srcDepth = constants.srcDepth.Get2D();
    Texture2D<float4> viewspaceNormals = constants.viewspaceNormals.Get2D();
    
    //if (u_quality == 0) // Low
    //{
    //    XeGTAO_MainPass(dispatchThreadID, 1, 2, SpatioTemporalNoise(dispatchThreadID, u_pushConstants.NoiseIndex), LoadNormal(dispatchThreadID), u_pushConstants, u_srcDepth, u_pointSamplerClamp, o_aoTerm, o_edges);
    //}
    //else if (u_quality == 1) // Medium
    {
        XeGTAO_MainPass(dispatchThreadID, 2, 2, SpatioTemporalNoise(dispatchThreadID, constants.constants.NoiseIndex), LoadNormal(dispatchThreadID, viewspaceNormals), constants.constants, srcDepth, constants.pointClampSampler.Get(), aoTerm, edges);
    }
    //else if (u_quality == 2) // High
    //{
    //    XeGTAO_MainPass(dispatchThreadID, 3, 3, SpatioTemporalNoise(dispatchThreadID, u_pushConstants.NoiseIndex), LoadNormal(dispatchThreadID), u_pushConstants, u_srcDepth, u_pointSamplerClamp, o_aoTerm, o_edges);
    //}
    //else if (u_quality == 3) // Ultra
    //{
    //    XeGTAO_MainPass(dispatchThreadID, 9, 3, SpatioTemporalNoise(dispatchThreadID, u_pushConstants.NoiseIndex), LoadNormal(dispatchThreadID), u_pushConstants, u_srcDepth, u_pointSamplerClamp, o_aoTerm, o_edges);
    //}
}