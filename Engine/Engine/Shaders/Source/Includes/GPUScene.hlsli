#pragma once

#include "Vertex.hlsli"
#include "Resources.hlsli"
#include "BoundingVolumes.hlsli"
#include "Transform.hlsli"

#define MAX_LOD_COUNT 8

struct GPUMaterial
{
    TTexture<float4> textures[16];
    TextureSampler samplers[16];
    
    uint textureCount;
    uint materialFlags;
    uint2 padding;
};

struct VertexTriangleCount
{
    uint vertexCount : 16;
    uint triangleCount : 16;
};

struct MeshletCone
{
    uint x : 8;
    uint y : 8;
    uint z : 8;
    uint cutoff : 8;
};

struct Meshlet
{
    VertexTriangleCount vertTriCount;
    uint meshId;
    uint dataOffset;
    MeshletCone cone;

    float3 boundingSphereCenter;
    float boundingSphereRadius;

    uint GetVertexCount()
    {
        return vertTriCount.vertexCount;
    }

    uint GetTriangleCount()
    {
        return vertTriCount.triangleCount;
    }

    uint GetVertexOffset()
    {
        return dataOffset;
    }

    uint GetIndexOffset()
    {
        return dataOffset + GetVertexCount();
    }

    float3 GetConeAxis()
    {
        return float3(cone.x / 127.f, cone.y / 127.f, cone.z / 127.f);
    }

    float GetConeCutoff()
    {
        return float(cone.cutoff / 127.f);
    }
};

struct GPUMesh
{
    TypedBuffer<float3> vertexPositionsBuffer;
    TypedBuffer<VertexMaterialData> vertexMaterialBuffer;
    TypedBuffer<VertexAnimationData> vertexAnimationInfoBuffer;
    TypedBuffer<uint16_t> vertexBoneInfluencesBuffer;

    TypedBuffer<float> vertexBoneWeightsBuffer; // Should be packed
    TypedBuffer<uint> meshletDataBuffer;
    TypedBuffer<Meshlet> meshletsBuffer;
    uint padding;
    
    uint vertexStartOffset;
    uint meshletCount;
    uint meshletStartOffset;
    uint meshletIndexStartOffset;
};

struct ObjectDrawData
{
    Transform transform;

    uint meshId;
    uint materialId;
    uint meshletStartOffset;
    uint entityId;
    
    BoundingSphere boundingSphere;

    uint isAnimated;
    uint boneOffset;
};

struct GPUScene
{
    UniformTypedBuffer<GPUMesh> meshesBuffer;
    UniformTypedBuffer<GPUMaterial> materialsBuffer;
    UniformTypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    UniformTypedBuffer<float4x4> bonesBuffer;
};