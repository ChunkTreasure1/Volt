#pragma once

#define NUM_MS_THREADS 64
#define NUM_AS_THREADS 32 // Wave size
#define NUM_MAX_OUT_TRIS 64
#define NUM_MAX_OUT_VERTS 64

#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "MeshletHelpers.hlsli"
#include "Matrix.hlsli"
#include "Packing.hlsli"

struct MeshAmplificationPayload
{
    uint drawId;
    uint meshletIndices[NUM_AS_THREADS];
};

struct DefaultPrimitiveOutput
{
    bool cullPrimitive : SV_CullPrimitive;
};

#ifndef OVERRIDE_DEFAULT_CONSTANTS
struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;
};
#endif

float4 TransformClipPosition(float4 position)
{
#if __VULKAN__
    position.y = -position.y;
    return position;
#else
    return position;
#endif
}

float3 GetNormal(GPUMesh mesh, uint vertexIndex)
{
    const VertexMaterialData materialData = mesh.vertexMaterialBuffer.Load(vertexIndex);

    uint2 octIntNormal;
    
    octIntNormal.x = (materialData.normal >> 0) & 0xFF;
    octIntNormal.y = (materialData.normal >> 8) & 0xFF;
    
    float2 octNormal = 0.f;
    octNormal.x = octIntNormal.x / 255.f;
    octNormal.y = octIntNormal.y / 255.f;
    return normalize(OctNormalDecode(octNormal));
}

float4x4 GetSkinningMatrix(GPUMesh mesh, uint vertexIndex, uint objectBoneOffset, UniformTypedBuffer<float4x4> bonesBuffer)
{
    VertexAnimationData animData = mesh.vertexAnimationInfoBuffer.Load(vertexIndex);

    float4x4 result = 0.f;

    result += mul(bonesBuffer.Load(objectBoneOffset + animData.influences[0]), animData.weights[0]);
    result += mul(bonesBuffer.Load(objectBoneOffset + animData.influences[1]), animData.weights[1]);
    result += mul(bonesBuffer.Load(objectBoneOffset + animData.influences[2]), animData.weights[2]);
    result += mul(bonesBuffer.Load(objectBoneOffset + animData.influences[3]), animData.weights[3]);

    return result;

    //VertexAnimationInfo animData = mesh.vertexAnimationInfoBuffer.Load(vertexIndex);
    //float4x4 result = 0.f;
    //
    //const uint16_t influenceCount = animData.influenceCount;
    //const uint16_t boneOffset = animData.boneOffset;
    //
    //for (uint i = 0; i < influenceCount; i++)
    //{
    //    const uint16_t influence = mesh.vertexBoneInfluencesBuffer.Load(boneOffset + i);
    //    const float weight = mesh.vertexBoneWeightsBuffer.Load(boneOffset + i);
    //    result += mul(bonesBuffer.Load(objectBoneOffset + influence), weight);
    //}
    //
    //return result;
}

// Culling
groupshared float3 m_vertexClipPositions[NUM_MS_THREADS];

void SetupCullingPositions(uint groupThreadId, float4 clipPosition, float2 renderSize)
{
    m_vertexClipPositions[groupThreadId] = float3((clipPosition.xy / clipPosition.w * 0.5f + 0.5f) * renderSize, clipPosition.w);
}

bool IsPrimitiveCulled(uint3 primitive)
{
    bool culled = false;
        
    float2 pa = m_vertexClipPositions[primitive.x].xy;
    float2 pb = m_vertexClipPositions[primitive.y].xy;
    float2 pc = m_vertexClipPositions[primitive.z].xy;

    // Backface and zero area culling.
    float2 eb = pb - pa;
    float2 ec = pc - pa;

    float2 bmin = min(pa, min(pb, pc));
    float2 bmax = max(pa, max(pb, pc));
    
    const float subPixelPrecision = 1.f / 256.f;
    
    culled = culled || (round(bmin.x - subPixelPrecision) == round(bmax.x) || round(bmin.y) == round(bmax.y + subPixelPrecision));
    culled = culled && (m_vertexClipPositions[primitive.x].z > 0.f && m_vertexClipPositions[primitive.y].z > 0.f && m_vertexClipPositions[primitive.z].z > 0.f); // Check that vertices are in front of perspective plane.

    return culled;
}