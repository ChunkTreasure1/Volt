#include "Buffers.hlsli"

struct Input
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;

    float3 worldPosition : WORLDPOSITION;
    uint currentLight : CURRENTLIGHT;
    
    float clipDepth : CLIPDEPTH;
    float depth : DEPTH;
};

float4 main(Input input) : SV_Target0
{
    clip(input.clipDepth);
    return input.depth;
}