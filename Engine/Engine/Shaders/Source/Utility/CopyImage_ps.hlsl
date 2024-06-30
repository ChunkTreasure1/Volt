#include "Vertex.hlsli"
#include "Resources.hlsli"

struct Constants
{
    UniformTexture<float3> color;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
    const float3 color = constants.color.Load2D(int3(input.position.xy, 0));

    Output output;
    output.output = color;

    return output;
}