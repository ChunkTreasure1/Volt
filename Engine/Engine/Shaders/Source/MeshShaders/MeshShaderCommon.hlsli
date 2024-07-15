#pragma once

#define NUM_MS_THREADS 64
#define NUM_MAX_OUT_TRIS 64
#define NUM_MAX_OUT_VERTS 64

#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Utility.hlsli"
#include "MeshletHelpers.hlsli"

#ifndef OVERRIDE_DEFAULT_CONSTANTS
struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;
};
#endif

float4 TransformSVPosition(float4 position)
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