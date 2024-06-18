#include "Resources.hlsli"
#include "../UI/UIVertex.hlsli"

struct Constants
{
    float4x4 viewProjection;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    uint widgetId : WIDGETID;
};

VSOutput MainVS(in UIVertex input)
{
    const Constants constants = GetConstants<Constants>();
    
    VSOutput output;
    output.position = mul(constants.viewProjection, input.position);
    output.texCoords = input.texCoords;
    output.widgetId = input.widgetId;

    return output;
}

struct PSOutput
{
    [[vt::r32ui]] uint color : SV_Target;
    [[vt::d32f]];
};

PSOutput MainPS(VSOutput input)
{
    PSOutput output;
    output.color = input.widgetId;

    return output;
}