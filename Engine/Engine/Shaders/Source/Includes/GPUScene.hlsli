#pragma once

#include "Vertex.hlsli"
#include "Resources.hlsli"

#define MAX_LOD_COUNT 8

struct GPUMaterial
{
    TextureT<float4> textures[16];
    uint samplers[16];
    
    uint textureCount;
    uint materialFlags;
    uint2 padding;
};

struct GPUMeshLOD
{
    uint indexCount;
    uint indexOffset;
};

struct Meshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
    
    uint objectId;
    uint meshId;
};

struct GPUMesh
{
    TypedBuffer<VertexPositionData> vertexPositionsBuffer;
    TypedBuffer<VertexMaterialData> vertexMaterialBuffer;
    TypedBuffer<VertexAnimationData> vertexAnimationBuffer;
    TypedBuffer<uint> indexBuffer;
    
    TypedBuffer<uint> meshletIndexBuffer;
    TypedBuffer<Meshlet> meshletsBuffer;
    
    uint vertexStartOffset;
    uint meshletCount;
    uint meshletStartOffset;
    uint meshletIndexStartOffset;
  
    uint lodCount;
    GPUMeshLOD lods[MAX_LOD_COUNT];
};

struct ObjectDrawData
{
    float4x4 transform;
    
    uint meshId;
    uint materialId;
    uint meshletStartOffset;
    uint padding;
    
    float boundingSphereRadius;
    float3 boundingSphereCenter;
};

struct GPUScene
{
    TypedBuffer<GPUMesh> meshesBuffer;
    TypedBuffer<GPUMaterial> materialsBuffer;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<Meshlet> meshletsBuffer;
};