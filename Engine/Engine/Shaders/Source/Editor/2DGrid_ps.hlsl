#include "Vertex.hlsli"
#include "Resources.hlsli"
#include "Utility.hlsli"

struct Constants
{
    float4x4 inverseViewProjection;
    float scale;
};

struct Output
{
    [[vt::rgba8]] float4 output : SV_Target0;
};

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();

    float4 worldPos = mul(constants.inverseViewProjection, float4(float2(input.uv.x * 2.f - 1.f, (1.f - input.uv.y) * 2.f - 1.f), 0.f, 1.f));
    worldPos /= worldPos.w;

    const float2 coords = worldPos.xy * constants.scale * 0.1;
    const float2 derivative = fwidth(coords);
    const float2 grid = abs(frac(coords - 0.5f) - 0.5f) / derivative;
 
    const float lin = min(grid.x, grid.y);
    const float minX = min(derivative.x, 1.f);
    const float minY = min(derivative.y, 1.f);

    float4 color = float4(0.2f, 0.2f, 0.2f, 1.f - min(lin, 1.f));

    const float thickness = 5;

    if (worldPos.x > -thickness * minX && worldPos.x < thickness * minX)
    {
        color.xyz = 0.1f;
    }

    if (worldPos.y > -thickness * minY && worldPos.y < thickness * minY)
    {
        color.xyz = 0.1f;
    }

    Output output; 
    output.output = color;

    return output;
}