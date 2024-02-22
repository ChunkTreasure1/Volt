#pragma once

#include "Vertex.hlsli"
#include "Resources.hlsli"

#define MAX_LOD_COUNT 8

struct GPUMaterial
{
    TTexture<float4> textures[16];
    TextureSampler samplers[16];
    
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
    uint padding;
    float clusterError;

    float parentError;
    float3 parentBoundingSphereCenter;    

    float3 boundingSphereCenter;
    float boundingSphereRadius;
    
    float4 cone;
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
    
    float3 boundingSphereCenter;
    float boundingSphereRadius;
};

struct GPUScene
{
    TypedBuffer<GPUMesh> meshesBuffer;
    TypedBuffer<GPUMaterial> materialsBuffer;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<Meshlet> meshletsBuffer;
};