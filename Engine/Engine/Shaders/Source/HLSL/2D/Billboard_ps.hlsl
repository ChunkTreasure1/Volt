#include "SamplerStates.hlsli"
#include "Bindless.hlsli"

struct Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoords : UV;
    uint textureIndex : TEXTUREINDEX;
    uint id : ID;
};

struct Output
{
    float4 color : SV_Target0;
    uint id : SV_Target1;
};

Output main(in Input input)
{
    float4 result = 0.f;
    
    if (input.textureIndex == 0)
    {
        result = input.color;
    }
    else
    {
        result = u_texture2DTable[NonUniformResourceIndex(input.textureIndex)].Sample(u_linearSampler, input.texCoords) * input.color;
    }
    
    Output output = (Output) 0;
    output.color = result;
    output.id = input.id;
    return output;
}