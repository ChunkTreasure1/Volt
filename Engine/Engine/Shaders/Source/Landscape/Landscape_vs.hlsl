#include "Resources.hlsli"
#include "Structures.hlsli"

struct Output
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    int instanceId : INSTANCE_ID;
};

struct Input
{
    int indexID : SV_VertexID;
    int instanceID : SV_InstanceID;
};

struct Constants
{
    TypedBuffer<ViewData> viewData;
    TypedBuffer<TextureT<float> > heightMaps;
    TextureSampler PointSampler;
    uint sideCellCount;
};

Texture2D<float> heightMap : register(t0);

Output main(in Input input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load(0);
  
    const float cellSideSize = 1.0f;
    
    const int vertexSideCount = 9;
    const float vertexSpacing = cellSideSize / (vertexSideCount - 1);

    const float localXVertexPosition = (input.indexID % vertexSideCount) * vertexSpacing;
    const float localZVertexPosition = (input.indexID / vertexSideCount) * vertexSpacing;
    
    const int xIndex = input.instanceID % constants.sideCellCount;
    const int zIndex = input.instanceID / constants.sideCellCount;
    
    const float localXCellPosition = (xIndex) * cellSideSize;
    const float localZCellPosition = (zIndex) * cellSideSize;
    
    const float2 localXZPosition = float2(localXCellPosition + localXVertexPosition, localZCellPosition + localZVertexPosition);
    
    const int heightmapIndex = xIndex + zIndex * constants.sideCellCount;
    TextureT<float> heightmap = constants.heightMaps.Load(heightmapIndex);    
    
    float chunkSideSize = constants.sideCellCount * cellSideSize;
    const float2 uv = localXZPosition / chunkSideSize;
    const float height = heightmap.SampleLevel2D(constants.PointSampler, uv, 0);
    
    float4 localPosition = float4(localXZPosition.x, sin(((input.indexID % vertexSideCount) + xIndex * (vertexSideCount - 1)) / 10.f) + cos(((input.indexID / vertexSideCount) + zIndex * (vertexSideCount - 1)) / 10.f), localXZPosition.y, 1.f);
    float4 worldPosition = localPosition;
    
    Output output;
    output.position = mul(viewData.projection, mul(viewData.view, worldPosition));
    output.color = float4(0.f, 1.f, 0.f, 1.f);
    output.normal = float3(0.f, 1.f, 0.f);
    output.normal = float3(0.f, 1.f, 0.f);
    output.instanceId = input.instanceID;
    
    return output;
}