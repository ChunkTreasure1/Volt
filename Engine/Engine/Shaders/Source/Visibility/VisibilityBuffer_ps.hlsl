#include "CommonBuffers.hlsli"

struct Output
{
    [[vt::r32ui]] uint visId : SV_Target0;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    uint visId : VISID;
};

Output main(Input input)
{
    Output output;
    output.visId = input.visId;
    return output;
}