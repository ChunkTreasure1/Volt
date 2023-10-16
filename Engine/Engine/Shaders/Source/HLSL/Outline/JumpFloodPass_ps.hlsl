#include "Common.hlsli"
#include "Defines.hlsli"
#include "SamplerStates.hlsli"

struct Input
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    float2 texelSize : TEXELSIZE;
    float2 UV[9] : UV0;
};

Texture2D u_texture : register(t0, SPACE_MATERIAL);

float ScreenDistance(float2 v, float2 texelSize)
{
    float ratio = texelSize.x / texelSize.y;
    v.x /= ratio;

    return dot(v, v);
}

void BoundsCheck(inout float2 xy, float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        xy = 1000.f;
    }
}

float4 main(Input input) : SV_Target
{
    float4 pixel = u_texture.Sample(u_pointSampler, input.UV[0]);

    for (uint j = 1; j <= 8; j++)
    {
        float4 n = u_texture.Sample(u_pointSampler, input.UV[j]);
        if (n.w != pixel.w)
        {
            n.xyz = 0.f;
        }

        n.xy += input.UV[j] - input.UV[0];

        BoundsCheck(n.xy, input.UV[j]);

        float dist = ScreenDistance(n.xy, input.texelSize);
        if (dist < pixel.z)
        {
            pixel.xyz = float3(n.xy, dist);
        }
    }

    return pixel;
}