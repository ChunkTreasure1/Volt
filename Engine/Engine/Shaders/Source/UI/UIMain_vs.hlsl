#include "Resources.hlsli"
#include "UIVertex.hlsli"

struct Constants
{
    float4x4 viewProjection;
    vt::TextureSampler linearSampler;
};

struct Output
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    float4 color : COLOR;
    uint imageHandle : IMAGEHANDLE;
};

Output main(in UIVertex input)
{
    const Constants constants = GetConstants<Constants>();
    
    Output output;
    output.position = mul(constants.viewProjection, input.position);
    output.color = input.color;
    output.texCoords = input.texCoords;
    output.imageHandle = input.imageHandle;

    return output;
}