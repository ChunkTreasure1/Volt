#include "Vertex.hlsli"
#include "Resources.hlsli"
#include "Structures.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint meshletIndex : MESHLET_INDEX;
};

struct Constants
{
    UniformBuffer<ViewData> viewData;
    TypedBuffer<VertexPositionData> positions;

    uint meshletIndex;
};

Output main(in uint vertexId : SV_VertexID)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();
    
    Output output;
    output.position = mul(viewData.projection, mul(viewData.view, float4(constants.positions.Load(vertexId).position, 1.f)));
    output.meshletIndex = constants.meshletIndex;
    return output;
}