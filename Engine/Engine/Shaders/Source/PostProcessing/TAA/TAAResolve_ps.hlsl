#include "Vertex.hlsli"
#include "Resources.hlsli"

struct Constants
{
    UniformTexture<float3> currentColor;
    UniformTexture<float3> previousColor;

    UniformTexture<float2> velocityTexture;

    TextureSampler pointSampler;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();

    const float2 velocity = constants.velocityTexture.Sample2D(constants.pointSampler, input.uv);

    float3 currentColor = constants.currentColor.Sample2D(constants.pointSampler, input.uv);
    float3 previousColor = constants.previousColor.Sample2D(constants.pointSampler, input.uv - velocity);

    Output output;
    output.output = currentColor * 0.1f + previousColor * 0.9f;;

    return output;
}