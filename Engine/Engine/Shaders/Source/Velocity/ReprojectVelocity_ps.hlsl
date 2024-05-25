#include "Vertex.hlsli"
#include "Resources.hlsli"

struct Constants
{
    float4x4 inverseViewProjection;
    float4x4 previousViewProjection;        
    float2 renderSize;
    float2 invRenderSize;
    float2 jitterOffset;

    UniformTexture<float> depthTexture;
    TextureSampler pointSampler;
};

struct Output
{
    [[vt::rg16f]] float2 output : SV_Target0;
};

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();

    const float pixelDepth = constants.depthTexture.Sample2D(constants.pointSampler, input.uv);
    const float x = input.uv.x * 2.f - 1.f;
    const float y = input.uv.y * 2.f - 1.f;

    const float4 projectedPos = float4(x, y, pixelDepth, 1.f);
    float4 worldPos = mul(constants.inverseViewProjection, projectedPos);
    worldPos.xyz /= worldPos.w;

    const float4 reprojectedPos = mul(constants.previousViewProjection, float4(worldPos.xyz, 1.f));
    const float2 reprojectedNDCPos = reprojectedPos.xy / reprojectedPos.w;
    const float2 reprojectedUV = reprojectedNDCPos * 0.5f + 0.5f;

    Output output;
    output.output = input.uv - reprojectedUV;

    return output;
}