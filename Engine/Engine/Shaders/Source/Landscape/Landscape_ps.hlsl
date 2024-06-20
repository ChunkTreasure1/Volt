#include "Utility.hlsli"

struct Output
{
    [[vt::rgba16f]] float4 finalColorOutput : SV_Target0;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    int instanceId : INSTANCE_ID;
};

Output main(Input input)
{
    Output output;
    // Normalize the normal vector
    float3 N = normalize(input.normal);

    // Normalize the light direction
    const float3 lightDirection = { 0.577f, 0.577f, 0.577f };
    float3 L = normalize(lightDirection);

    // Diffuse lighting factor (how much the surface faces the light)
    float diffuseFactor = max(0.0, dot(N, L));

    // Final color calculation
    const float3 objectColor = { 0.0f, 1.0f, 0.0f };
    const float3 lightColor = { 1.0f, 1.0f, 1.0f };
    const float ambientIntensity = 0.1f;
    
    float3 diffuseColor = diffuseFactor * lightColor * objectColor;
    float3 ambientColor = ambientIntensity * objectColor;

    
    float3 finalColor = GetRandomColor(input.instanceId);
    output.finalColorOutput = float4(finalColor, 1);
    return output;
}