#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"
#include "Vertex.hlsli"

#include "PBR.hlsli"

struct Output
{
    float4 color : SV_Target0;
};

Output main(DefaultPixelInput input)
{
    Output output = (Output) 0;
    output.color = input.paintedColor;
    return output;
}