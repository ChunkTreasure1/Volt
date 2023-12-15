#include <Common.hlsli>
#include <Buffers.hlsli>
#include <Utility.hlsli>

struct Input
{
    float4 position : SV_Position;
    float3 nearPoint : NEARPOINT;
    float3 farPoint : FARPOINT;
};

struct Output
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

float4 Grid(float3 position, float scale)
{
    const float2 coords = position.xz * scale;
    const float2 derivative = fwidth(coords);
    const float2 grid = abs(frac(coords - 0.5f) - 0.5f) / derivative;
	
    const float lin = min(grid.x, grid.y);
    const float minZ = min(derivative.y, 1.f);
    const float minX = min(derivative.x, 1.f);

    float4 color = float4(0.2f, 0.2f, 0.2f, 1.f - min(lin, 1.f));

	// Z axis
    if (position.x > -1.f * minX && position.x < 1.f * minX)
    {
        color.z = 1.f;
    }

	// X axis
    if (position.z > -1.f * minZ && position.z < 1.f * minZ)
    {
        color.x = 1.f;
    }

    return color;
}

float ComputeDepth(float3 position)
{
    float4 clipSpacePos = mul(u_cameraData.projection, mul(u_cameraData.view, float4(position, 1.f)));
    return clipSpacePos.z / clipSpacePos.w;
}

float ComputeLinearDepth(float3 position)
{
    float4 clipSpacePos = mul(u_cameraData.projection, mul(u_cameraData.view, float4(position, 1.f)));
    float clipSpaceDepth = (clipSpacePos.z / clipSpacePos.w) * 2.f - 1.f;
    
    float linearDepth = (2.f * u_cameraData.nearPlane * u_cameraData.farPlane) / (u_cameraData.farPlane + u_cameraData.nearPlane - clipSpaceDepth * (u_cameraData.farPlane - u_cameraData.nearPlane));
    return linearDepth / u_cameraData.farPlane;
}

Output main(Input input)
{
    const float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
    const float3 position = input.nearPoint + t * (input.farPoint - input.nearPoint);

    const float linearDepth = ComputeLinearDepth(position);
    const float fading = max(0.f, (0.5f - linearDepth));

    Output output;
    output.color = Grid(position, 1.f) * float(t > 0);
    output.color.a *= fading;
    output.depth = ComputeDepth(position);

    return output;
}