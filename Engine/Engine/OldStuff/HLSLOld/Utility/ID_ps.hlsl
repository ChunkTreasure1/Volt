#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"

struct Input
{
    float4 position : SV_Position;
    STAGE_VARIABLE(uint, objectID, OBJECTID, 3);
};

struct Output
{
    uint id : SV_Target0;
};

Output main(Input input)
{
    Output output = (Output) 0;
    output.id = input.objectID;
    
    return output;
}