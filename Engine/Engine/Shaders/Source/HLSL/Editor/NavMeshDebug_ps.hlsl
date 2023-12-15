#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"
#include "Vertex.hlsli"

#include "PBR.hlsli"

struct Output
{
    float4 accumulation : SV_Target0;
    float revealage : SV_Target1;
};

Output main(DefaultPixelInput input)
{
    const float4 color = float4(0.f, 0.57f, 1.f, 0.9f);
    
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 *
                         pow(1.0 - input.position.z * 0.9, 3.0), 1e-2, 3e3);
    
    Output output = (Output) 0;
    output.accumulation = float4(color.rgb * color.a, color.a) * weight;
    output.revealage = color.a;
    return output;
}