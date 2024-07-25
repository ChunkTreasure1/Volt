#include "Vertex.hlsli"
#include "Resources.hlsli"

struct Constants
{
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

Output MainPS(FullscreenTriangleVertex input)
{
}