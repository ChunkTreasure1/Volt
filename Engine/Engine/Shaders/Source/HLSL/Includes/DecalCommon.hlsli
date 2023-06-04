#include "Buffers.hlsli"
#include "Vertex.hlsli"
#include "Matrix.hlsli"
#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"
#include "Utility.hlsli"


#ifndef DECAL_COMMON

struct DecalOutput
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION;
    float2 texCoords : TEXCOORD;
};

struct PushConstants
{
    float4x4 transform;
    uint2 targetSize;
    uint materialIndex;
    float randomValue;
    float timeSinceCreation;
};

[[vk::push_constant]] PushConstants u_materialData;

Texture2D<float4> u_depthTexture : register(t0, SPACE_MATERIAL);

float3x3 CalculateDecalTBN(float3 worldPosition, float3 viewVector, float2 uv)
{
    const float3 ddxWp = ddx(worldPosition);
    const float3 ddyWp = ddy(worldPosition);

    const float3 normal = normalize(cross(ddyWp, ddxWp));

    const float3 dp1 = ddx(viewVector);
    const float3 dp2 = ddy(viewVector);

    const float2 duv1 = ddx(uv);
    const float2 duv2 = ddy(uv);

    const float3 dp2perp = cross(dp2, normal);
    const float3 dp1perp = cross(normal, dp1);

    const float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    const float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    const float invMax = rsqrt(max(dot(T, T), dot(B, B)));
    const float3x3 TBN = transpose(float3x3(T * invMax, B * invMax, -1.f * normal));

    return TBN;
}

#endif