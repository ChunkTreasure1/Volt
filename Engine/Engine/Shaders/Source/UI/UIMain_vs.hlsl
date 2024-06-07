#include "Resources.hlsli"
#include "UIVertex.hlsli"

struct Constants
{
    float4x4 viewProjection;
};

struct Output
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    uint imageHandle : IMAGEHANDLE;
};

Output main(in UIVertex input)
{
    const Constants constants = GetConstants<Constants>();
    
    Output output;
    output.position = mul(constants.viewProjection, input.position);
    output.texCoords = input.texCoords;
    output.imageHandle = input.imageHandle;

    return output;
}