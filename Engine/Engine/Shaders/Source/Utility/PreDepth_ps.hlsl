#include "CommonBuffers.hlsli"

struct Output
{
    [[vt::rgba16f]] float4 normal : SV_Target;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
};

Output main(Input input)
{
    float3 normal = normalize(input.normal);
    
    Output output;
    output.normal = float4(normal, 1.f);    

    return output;
}