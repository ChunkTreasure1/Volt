#pragma once

struct VertexMaterialData
{
    uint normal;
    float tangent;
    float16_t2 texCoords;
};

struct VertexAnimationData
{
    uint16_t4 influences;
    float16_t4 weights;
};

struct VertexPositionData
{
    float3 position;
};