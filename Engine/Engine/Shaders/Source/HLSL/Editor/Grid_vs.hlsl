#include <Common.hlsli>
#include <Buffers.hlsli>
#include <Matrix.hlsli>

struct Output
{
    float4 position : SV_Position;
    float3 nearPoint : NEARPOINT;
    float3 farPoint : FARPOINT;
};

float3 UnprojectPoint(float x, float y, float z)
{
    float4 unprojectedPoint = mul(u_cameraData.inverseView, mul(inverse(u_cameraData.nonReversedProj), float4(x, y, z, 1.f)));
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

Output main(const uint vertexIndex : SV_VertexID)
{
    const float3 gridPlane[6] =
    {
        float3(1, 1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
        float3(-1, -1, 0), float3(1, 1, 0), float3(1, -1, 0)
    };
    
    float4 position = float4(gridPlane[vertexIndex], 1.f);

    Output output;
    output.position = position;
    output.nearPoint = UnprojectPoint(position.x, position.y, 0.f);
    output.farPoint = UnprojectPoint(position.x, position.y, 1.f);
    return output;
}