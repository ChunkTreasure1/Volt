#pragma once

struct VertexMaterialData
{
    uint normal;
    float tangent;
    float16_t2 texCoords;
};

struct VertexAnimationData
{
    uint4 influences;
    float4 weights;
};

struct VertexAnimationInfo
{
    uint16_t influenceCount;
    uint16_t boneOffset;  
};

struct VertexPositionData
{
    float3 position;
};

struct FullscreenTriangleVertex
{
    float4 position : SV_Position;
    float2 uv : UV;
};