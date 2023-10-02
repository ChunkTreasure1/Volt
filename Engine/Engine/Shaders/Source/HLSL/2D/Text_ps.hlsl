#include <Common.hlsli>
#include <Material.hlsli>
#include <SamplerStates.hlsli>

struct Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoords : TEXCOORD;
    uint textureIndex : TEXTUREINDEX;
};

Texture2D u_textures[32] : register(t0);

float Median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float ScreenPxRange(float2 texSize, float2 texCoords)
{
    float pxRange = 2.f;
    float2 unitRange = pxRange / texSize;
    float2 screenTexSize = 1.f / fwidth(texCoords);

    return max(0.5f * dot(unitRange, screenTexSize), 1.f);
}

float4 main(Input input) : SV_Target
{
    const float3 msd = SampleTexture(input.textureIndex, u_linearSampler, input.texCoords).xyz;
    const uint2 texSize = GetTextureDimensions(input.textureIndex);
    
    float4 bgColor = float4(input.color.xyz, 0.f);
    float4 fgColor = input.color;

    float sd = Median(msd.r, msd.g, msd.b);
    float screenPxDistance = ScreenPxRange((float2) texSize, input.texCoords) * (sd - 0.5f);
    float opacity = clamp(screenPxDistance + 0.5f, 0.f, 1.f);

    if (opacity < 0.5f)
    {
        discard;
    }
    
    return lerp(bgColor, fgColor, opacity);
}