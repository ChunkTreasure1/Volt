#include "Resources.hlsli"
#include "Structures.hlsli"


#define SIDE_CELL_COUNT 1

struct Output
{
    float4 position : SV_Position;
    float4 color : COLOR;
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
    
    const float localXCellPosition = (input.instanceID % SIDE_CELL_COUNT) * cellSideSize;
    const float localZCellPosition = (input.instanceID / SIDE_CELL_COUNT) * cellSideSize;
    
    const float2 localXZPosition = float2(localXCellPosition + localXVertexPosition, localZCellPosition + localZVertexPosition);
    
    const int heightmapIndex = 0;
    TextureT<float> heightmap = constants.heightMaps.Load(heightmapIndex);    
    
    float chunkSideSize = SIDE_CELL_COUNT * cellSideSize;
    const float2 uv = localXZPosition / chunkSideSize;
    const float height = heightmap.SampleLevel2D(constants.PointSampler.Get(), uv, 0);
    
    float4 localPosition = float4(localXZPosition.x, height, localXZPosition.y, 1.f);
    float4 worldPosition = localPosition;
    
    Output output;
    output.position = localPosition; //mul(viewData.projection, mul(viewData.view, worldPosition));
    output.color = float4(0.f, 1.f, 0.f, 1.f);
    
    return output;
}