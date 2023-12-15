#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;

    float3 worldPosition : WORLDPOSITION;
    uint currentLight : CURRENTLIGHT;
};

struct PushConstants
{
    uint pointLightIndex;
};

[[vk::push_constant]] PushConstants u_pushConstants;

[maxvertexcount(18)]
void main(triangle float4 input[3] : POSITION, inout TriangleStream<Output> output)
{
    const uint faceCount = 6;
    
    const PointLight pointLight = u_pointLights[u_pushConstants.pointLightIndex];
    
    for (uint face = 0; face < faceCount; face++)
    {
        Output result;
        result.target = face;
        
        for (uint i = 0; i < 3; i++)
        {
            result.position = mul(pointLight.viewProjectionMatrices[face], input[i]);
            result.worldPosition = input[i].xyz;
            result.currentLight = u_pushConstants.pointLightIndex;
            output.Append(result);
        }
        
        output.RestartStrip();
    }
}