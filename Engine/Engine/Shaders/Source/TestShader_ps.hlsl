#include "Utility.hlsli"

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 albedo : SV_Target;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    uint meshletIndex : MESHLET_INDEX;

    uint primitiveId : SV_PrimitiveID;
};

Output main(Input input)
{
    Output output;
    output.albedo = GetRandomColor(input.meshletIndex);    

    return output;
}