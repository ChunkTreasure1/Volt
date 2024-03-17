#include "Resources.hlsli"
#include "Structures.hlsli"

struct Output
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct Vertex
{
    float3 position : POSITION;
    [[vt::half3]] [[vt::instance]] float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct Constants
{
    UniformBuffer<ViewData> viewData;
};

Output main(in Vertex input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();
    
    float4 worldPosition = float4(input.position, 1.f);
    
    Output output;
    output.position = mul(viewData.projection, mul(viewData.view, worldPosition));
    output.color = float4(0.f, 1.f, 0.f, 1.f);
    
    return output;
}