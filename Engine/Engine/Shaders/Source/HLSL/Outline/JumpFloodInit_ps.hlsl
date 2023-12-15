#include "Common.hlsli"
#include "SamplerStates.hlsli"
#include "Vertex.hlsli"

Texture2D u_texture : register(t0, SPACE_MATERIAL);

float ScreenDistance(float2 v, float2 texelSize)
{
    float ratio = texelSize.x / texelSize.y;
    v.x /= ratio;

    return dot(v, v);
}
 
float4 main(DefaultFullscreenTriangleVertex input) : SV_Target
{
    float4 color = u_texture.Sample(u_pointSampler, input.texCoords);

    uint2 texSize;
    u_texture.GetDimensions(texSize.x, texSize.y);

    float2 texelSize = float2(1.f / (float) texSize.x, 1.f / (float) texSize.y);

    float4 result;
    result.xy = float2(100.f, 100.f);
    result.z = ScreenDistance(result.xy, texelSize);

    result.w = color.a > 0.5f ? 1.f : 0.f;
    return result;
}