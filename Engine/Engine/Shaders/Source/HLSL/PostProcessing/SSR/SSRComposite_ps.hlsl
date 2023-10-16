#include "Vertex.hlsli"
#include "SamplerStates.hlsli"

#include "Utility.hlsli"

Texture2D<float4> u_sceneColor : register(t0, SPACE_MATERIAL);

float4 main(DefaultFullscreenTriangleVertex input) : SV_Target0
{
    float4 color = u_sceneColor.Sample(u_pointSampler, input.texCoords);
    
    if (color.a > 0.5f)
    {
        discard;
    }
    
    return color;
}