#include "Resources.hlsli"
#include "PushConstant.hlsli"

struct Vertex
{
    float3 position : POSITION;
};

struct VSOut
{
    float4 position : SV_Position;
};

struct Constants
{
    float4x4 viewProjection;
};

struct PushConstants
{
    float3 position;
    float3 scale;

    float data;
};

PUSH_CONSTANT(PushConstants, u_pushConstants);

VSOut MainVS(in Vertex input)
{
    const Constants constants = GetConstants<Constants>();

    VSOut result;
    result.position = mul(constants.viewProjection, float4(input.position * u_pushConstants.scale + u_pushConstants.position, 1.f));

    return result;
}

struct ColorOutput
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
    [[vt::d32f]];
};

ColorOutput MainPS(VSOut input)
{
    const Constants constants = GetConstants<Constants>();
    
    ColorOutput result;
    result.output = float3(1.f, 1.f, 0.f);
    
    return result;
}