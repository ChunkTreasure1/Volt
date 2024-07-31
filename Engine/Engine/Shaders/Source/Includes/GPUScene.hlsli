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
    
    BoundingSphere boundingSphere;

    uint vertexStartOffset;
    uint meshletCount;
    uint meshletStartOffset;
    uint meshletIndexStartOffset;
};

struct GPUSDFBrick
{
    float3 min;
    float3 max;
    float3 localCoords;
};

struct GPUMeshSDF
{
    TTexture<float> sdfTexture;
    float3 size;

    float3 min;
    float3 max;
    
    TypedBuffer<GPUSDFBrick> bricksBuffer;
    uint brickCount;
};

struct PrimitiveDrawData
{
    Transform transform;

    uint meshId;
    uint materialId;
    uint meshletStartOffset;
    uint entityId;
    
    uint isAnimated;
    uint boneOffset;
};

struct SDFPrimitiveDrawData
{
    Transform transform;

    uint meshSDFId;
    uint primitiveId;
};

struct GPUScene
{
    UniformTypedBuffer<GPUMesh> meshesBuffer;
    UniformTypedBuffer<GPUMeshSDF> sdfMeshesBuffer;
    UniformTypedBuffer<GPUMaterial> materialsBuffer;
    UniformTypedBuffer<PrimitiveDrawData> primitiveDrawDataBuffer;
    UniformTypedBuffer<SDFPrimitiveDrawData> sdfPrimitiveDrawDataBuffer;
    UniformTypedBuffer<float4x4> bonesBuffer;
};