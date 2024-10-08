#include "Resources.hlsli"
#include "Structures.hlsli"
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

struct Input
{
    float4 position : SV_Position;
    float3 samplePosition : SAMPLE_POSITION;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

Output main(Input input)
{
    const Constants constants = GetConstants<Constants>();

    float3 result = constants.environmentTexture.SampleLevel(constants.linearSampler, input.samplePosition / 100.f, constants.lod) * constants.intensity;
    
    Output output;
    output.output = result;
    return output;
}