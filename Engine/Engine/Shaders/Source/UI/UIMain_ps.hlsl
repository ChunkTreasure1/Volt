#include "Resources.hlsli"
#include "CommonBuffers.hlsli"

struct Constants
{
    float4x4 viewProjection;
    vt::TextureSampler linearSampler;
};

struct Output
{
    [[vt::rgba8]] float4 color : SV_Target;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    float4 color : COLOR;
    uint imageHandle : IMAGEHANDLE;
};

Output main(Input input)
{
    const Constants constants = GetConstants<Constants>();

    vt::Tex2D<float4> texture = (vt::Tex2D<float4>)input.imageHandle;

    Output output;
    output.color = texture.Sample(constants.linearSampler, input.texCoords);
    output.color.rgb *= input.color.rgb;
    output.color.a = input.color.a;

    return output;
}