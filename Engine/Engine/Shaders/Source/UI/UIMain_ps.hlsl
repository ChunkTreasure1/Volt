#include "CommonBuffers.hlsli"

struct Output
{
    [[vt::rgba8]] float4 color : SV_Target;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    uint imageHandle : IMAGEHANDLE;
};

Output main(Input input)
{
    Output output;
    output.color = 1.f;

    return output;
}