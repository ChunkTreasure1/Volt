#include "Vertex.hlsli"
#include "SamplerStates.hlsli"

#define XE_GTAO_OCCLUSION_TERM_SCALE                    (1.5f)      // for packing in UNORM (because raw, pre-denoised occlusion term can overshoot 1 but will later average out to 1)

Texture2D<uint> u_gtaoOutput : register(t0, SPACE_MATERIAL);

float4 main(DefaultFullscreenTriangleVertex input) : SV_Target0
{
    const float ao = (u_gtaoOutput.Load(uint3(input.position.xy, 0)).x >> 24) / 255.f;
    const float result = min(ao * XE_GTAO_OCCLUSION_TERM_SCALE, 1.f);

    return result;

}