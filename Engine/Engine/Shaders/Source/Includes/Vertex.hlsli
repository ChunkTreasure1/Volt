#pragma once

struct VertexMaterialData
{
    uint normal;
    float tangent;
    float16_t2 texCoords;
};

struct VertexAnimationInfo
{
    uint32_t boneOffset;  
    uint16_t influenceCount;
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