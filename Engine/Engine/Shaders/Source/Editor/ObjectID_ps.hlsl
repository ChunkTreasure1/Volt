#include "CommonBuffers.hlsli"

struct Output
{
    [[vt::r32ui]] uint objectId : SV_Target;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    uint objectId : OBJECT_ID;
};

Output main(Input input)
{
    Output output;
    output.objectId = input.objectId;

    return output;
}