#pragma once

#include "Vertex.hlsli"
#include "Resources.hlsli"

#define MAX_LOD_COUNT 8

struct GPUMaterial
{
    uint textures[16];
    
    uint textureCount;
    uint materialFlags;
    uint2 padding;
};

struct GPUMeshLOD
{
    uint indexCount;
    uint indexOffset;
};

struct GPUMesh
{
    TypedBuffer<VertexPositionData> vertexPositionsBuffer;
    TypedBuffer<VertexMaterialData> vertexMaterialBuffer;
    TypedBuffer<VertexAnimationData> vertexAnimationBuffer;
    TypedBuffer<uint> indexBuffer;
    
    uint vertexStartOffset;
    
    uint16_t lodCount;
    GPUMeshLOD lods[MAX_LOD_COUNT];
};

struct ObjectDrawData
{
    float4x4 transform;
    uint meshId;
    uint materialId;
};

struct GPUScene
{
    TypedBuffer<GPUMesh> meshesBuffer;
    TypedBuffer<GPUMaterial> materialsBuffer;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
};