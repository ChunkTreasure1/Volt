#include "Vertex.hlsli"
#include "Resources.hlsli"

struct Constants
{
    UniformTexture<float3> currentColor;
    UniformTexture<float3> previousColor;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();

    float3 currentColor = constants.currentColor.Load2D(int3(input.position.xy, 0));
    float3 previousColor = constants.previousColor.Load2D(int3(input.position.xy, 0));

    Output output;
    output.output = currentColor * 0.1f + previousColor * 0.9f;;

    return output;
}