#pragma once

#include "Resources.hlsli"
#include "Packing.hlsli"

struct MaterialData
{
    float2 texCoords[3];
    float3 normals[3];
    float3 tangents[3]; 
};

struct PositionData
{
    float3 positions[3];
};

uint3 LoadTriangleIndices(TypedBuffer<uint> meshletDataBuffer, uint offset, uint vertexOffset, uint3 meshletTriIndices)
{
    uint3 result;
    result.x = meshletDataBuffer.Load(offset + meshletTriIndices.x) + vertexOffset;
    result.y = meshletDataBuffer.Load(offset + meshletTriIndices.y) + vertexOffset;
    result.z = meshletDataBuffer.Load(offset + meshletTriIndices.z) + vertexOffset;

    return result;
}

PositionData LoadVertexPositions(TypedBuffer<float3> positionsBuffer, uint3 triIndices)
{
    PositionData result;
    result.positions[0] = positionsBuffer.Load(triIndices.x);
    result.positions[1] = positionsBuffer.Load(triIndices.y);
    result.positions[2] = positionsBuffer.Load(triIndices.z);

    return result;
}

MaterialData LoadVertexMaterialData(TypedBuffer<VertexMaterialData> buffer, uint3 triIndices)
{
    const VertexMaterialData material0 = buffer.Load(triIndices.x);
    const VertexMaterialData material1 = buffer.Load(triIndices.y);
    const VertexMaterialData material2 = buffer.Load(triIndices.z);

    MaterialData result;
    result.texCoords[0] = material0.texCoords;
    result.texCoords[1] = material1.texCoords;
    result.texCoords[2] = material2.texCoords;

    result.normals[0] = DecodeNormal(material0.normal);
    result.normals[1] = DecodeNormal(material1.normal);
    result.normals[2] = DecodeNormal(material2.normal);

    result.tangents[0] = DecodeTangent(result.normals[0], material0.tangent);
    result.tangents[1] = DecodeTangent(result.normals[1], material0.tangent);
    result.tangents[2] = DecodeTangent(result.normals[2], material0.tangent);

    return result;
}