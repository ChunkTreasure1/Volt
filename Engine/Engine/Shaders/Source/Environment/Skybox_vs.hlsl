#include "Resources.hlsli"
#include "Structures.hlsli"
#include "Matrix.hlsli"
#include "Vertex.hlsli"

struct Constants
{
    vt::TypedBuffer<VertexPositionData> vertexPositions;
    vt::UniformBuffer<ViewData> viewData;

    vt::TexCube<float3> environmentTexture;
    vt::TextureSampler linearSampler;

    float lod;
    float intensity;
};

struct Output
{
    float4 position : SV_Position;
    float3 samplePosition : SAMPLE_POSITION;
};

Output main(in uint vertexId : SV_VertexID)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const VertexPositionData vertexPosition = constants.vertexPositions.Load(vertexId);

    const float3x3 view = (float3x3)viewData.view;
    float4x4 viewRotation = IDENTITY_MATRIX;
    viewRotation[0].xyz = view[0];
    viewRotation[1].xyz = view[1];
    viewRotation[2].xyz = view[2];

    const float4 clipPos = mul(viewData.projection, mul(viewRotation, float4(vertexPosition.position, 1.f)));
    const float3 dir = normalize(float3(viewData.view[0][2], viewData.view[1][2], viewData.view[1][2]));
    
    Output output;
    output.position = clipPos;
    output.samplePosition = vertexPosition.position;

    return output;
}