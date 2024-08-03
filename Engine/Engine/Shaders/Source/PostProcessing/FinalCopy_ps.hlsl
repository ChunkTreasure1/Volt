#include "Vertex.hlsli"
#include "Resources.hlsli"
#include "ACES.hlsli"
#include "Utility.hlsli"

struct Constants
{
    vt::UniformTex2D<float3> finalColor;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
    float3 currentColor = constants.finalColor.Load(int3(input.position.xy, 0));

    currentColor = ACESFitted(currentColor);

    Output output;
    output.output = LinearToSRGB(currentColor);

    return output;
}