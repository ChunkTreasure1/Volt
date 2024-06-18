#pragma once

#include "Vertex.hlsli"
#include "Resources.hlsli"
#include "BoundingVolumes.hlsli"

#define MAX_LOD_COUNT 8

struct GPUMaterial
{
    TTexture<float4> textures[16];
    TextureSampler samplers[16];
    
    uint textureCount;
    uint materialFlags;
    uint2 padding;
};

struct Meshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
    
    uint objectId;
    uint meshId;
    uint2 padding;
    
    float3 boundingSphereCenter;
    float boundingSphereRadius;
    
    float4 cone;
};

struct GPUMesh
{
    TypedBuffer<VertexPositionData> vertexPositionsBuffer;
    TypedBuffer<VertexMaterialData> vertexMaterialBuffer;
    TypedBuffer<VertexAnimationInfo> vertexAnimationInfoBuffer;
    TypedBuffer<uint16_t> vertexBoneInfluencesBuffer;

    TypedBuffer<uint> vertexBoneWeightsBuffer; // packed: one element is 4 weights
    TypedBuffer<uint> indexBuffer;
    TypedBuffer<uint> meshletIndexBuffer;
    TypedBuffer<Meshlet> meshletsBuffer;
    
    uint vertexStartOffset;
    uint meshletCount;
    uint meshletStartOffset;
    uint meshletIndexStartOffset;
};

struct ObjectDrawData
{
    float4x4 transform;
    
    uint meshId;
    uint materialId;
    uint meshletStartOffset;
    uint entityId;
    
    BoundingSphere boundingSphere;
};

struct GPUScene
{
    TypedBuffer<GPUMesh> meshesBuffer;
    TypedBuffer<GPUMaterial> materialsBuffer;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<Meshlet> meshletsBuffer;
};