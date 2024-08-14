#pragma once

#include "Vertex.hlsli"
#include "Resources.hlsli"
#include "BoundingVolumes.hlsli"
#include "Transform.hlsli"

#define MAX_LOD_COUNT 8

struct GPUMaterial
{
    vt::Tex2D<float4> textures[16];
    vt::TextureSampler samplers[16];
    
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
    vt::TypedBuffer<float3> vertexPositionsBuffer;
    vt::TypedBuffer<VertexMaterialData> vertexMaterialBuffer;
    vt::TypedBuffer<VertexAnimationData> vertexAnimationInfoBuffer;
    vt::TypedBuffer<uint16_t> vertexBoneInfluencesBuffer;

    vt::TypedBuffer<float> vertexBoneWeightsBuffer; // Should be packed
    vt::TypedBuffer<uint> meshletDataBuffer;
    vt::TypedBuffer<Meshlet> meshletsBuffer;
    
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
    vt::Tex3D<float> sdfTexture;
    float3 size;

    float3 min;
    float3 max;
    
    vt::TypedBuffer<GPUSDFBrick> bricksBuffer;
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
    vt::UniformTypedBuffer<GPUMesh> meshesBuffer;
    vt::UniformTypedBuffer<GPUMeshSDF> sdfMeshesBuffer;
    vt::UniformTypedBuffer<GPUMaterial> materialsBuffer;
    vt::UniformTypedBuffer<PrimitiveDrawData> primitiveDrawDataBuffer;
    vt::UniformTypedBuffer<SDFPrimitiveDrawData> sdfPrimitiveDrawDataBuffer;
    vt::UniformTypedBuffer<float4x4> bonesBuffer;
};