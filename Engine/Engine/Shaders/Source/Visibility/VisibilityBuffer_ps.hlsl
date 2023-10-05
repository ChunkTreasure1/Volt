#include "CommonBuffers.hlsli"

struct Output
{
    [[vt::rg32ui]] uint2 visId : SV_Target;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    uint2 visId : VISID;
};

Output main(Input input)
{
    Output output;
    output.visId = input.visId;
    return output;
}