#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"

struct Output
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;

    float3 worldPosition : WORLDPOSITION;
    uint currentLight : CURRENTLIGHT;
    
    float clipDepth : CLIPDEPTH;
    float depth : DEPTH;
};

struct PushConstants
{
    uint pointLightIndex;
};

[[vk::push_constant]] PushConstants u_pushConstants;

[maxvertexcount(6)]
void main(triangle float4 input[3] : POSITION, inout TriangleStream<Output> output)
{
    const PointLight pointLight = u_pointLights[u_pushConstants.pointLightIndex];
    
    // Front
    {
        Output result;
        result.target = 0;
        
        for (uint i = 0; i < 3; i++)
        {
            result.position = mul(pointLight.viewProjectionMatrices[0], input[i]);
            result.position /= result.position.w;
            
            float vectorLength = length(result.position.xyz);
            result.position /= vectorLength;
            
            result.clipDepth = result.position.z;
            
            // calc "normal" on intersection, by adding the 
	        // reflection-vector(0,0,1) and divide through 
	        // his z to get the texture coords
            result.position.x /= result.position.z + 1.0f;
            result.position.y /= result.position.z + 1.0f;
            
            result.position.z = (vectorLength - u_cameraData.nearPlane) / (u_cameraData.farPlane - u_cameraData.nearPlane);
            result.position.w = 1.f;
            
            result.depth = result.position.z;
            
            result.worldPosition = input[i].xyz;
            result.currentLight = u_pushConstants.pointLightIndex;
            output.Append(result);
        }
        
        output.RestartStrip();
    }
    
    // Back
    {
        Output result;
        result.target = 1;
        
        for (uint i = 0; i < 3; i++)
        {
            result.position = mul(pointLight.viewProjectionMatrices[1], input[i]);
            result.position /= result.position.w;
            
            float vectorLength = length(result.position.xyz);
            result.position /= vectorLength;
            
            result.clipDepth = result.position.z;
            
            // calc "normal" on intersection, by adding the 
	        // reflection-vector(0,0,1) and divide through 
	        // his z to get the texture coords
            result.position.x /= result.position.z + 1.0f;
            result.position.y /= result.position.z + 1.0f;
            
            result.position.z = (vectorLength - u_cameraData.nearPlane) / (u_cameraData.farPlane - u_cameraData.nearPlane);
            result.position.w = 1.f;
            
            result.depth = result.position.z;
            
            result.worldPosition = input[i].xyz;
            result.currentLight = u_pushConstants.pointLightIndex;
            output.Append(result);
        }
        
        output.RestartStrip();
    }
}