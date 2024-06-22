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
    TypedBuffer<VertexAnimationData> vertexAnimationInfoBuffer;
    TypedBuffer<uint16_t> vertexBoneInfluencesBuffer;

    TypedBuffer<float> vertexBoneWeightsBuffer; // Should be packed
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

    uint isAnimated;
    uint boneOffset;
    uint2 padding;
};

struct GPUScene
{
    UniformTypedBuffer<GPUMesh> meshesBuffer;
    UniformTypedBuffer<GPUMaterial> materialsBuffer;
    UniformTypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    UniformTypedBuffer<Meshlet> meshletsBuffer;
    UniformTypedBuffer<float4x4> bonesBuffer;
};