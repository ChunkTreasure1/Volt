#pragma once

#include "Defines.hlsli"
#include "GPUScene.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"
#include "MeshletHelpers.hlsli"
#include "Matrix.hlsli"

#ifndef OVERRIDE_DEFAULT_CONSTANTS

struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;
};

#endif

static Constants m_constants;
static Meshlet m_meshlet;
static ObjectDrawData m_objectDrawData;
static GPUMesh m_gpuMesh;
static uint m_index;

struct DefaultInput
{
    uint vertexId : SV_VertexID;
    
    const uint GetTriangleID()
    {
        return UnpackTriangleID(vertexId);
    }
     
    const uint GetMeshletID()
    {
        return UnpackMeshletID(vertexId);
    }
    
    const uint GetPackedPrimitveID()
    {
        return vertexId;
    }

    void Initialize()
    {
        m_constants = GetConstants<Constants>();
        m_meshlet = m_constants.gpuScene.meshletsBuffer.Load(GetMeshletID());
        m_objectDrawData = m_constants.gpuScene.objectDrawDataBuffer.Load(m_meshlet.objectId);
        m_gpuMesh = m_constants.gpuScene.meshesBuffer.Load(m_meshlet.meshId);
    
        m_index = m_gpuMesh.meshletIndexBuffer.Load(m_gpuMesh.meshletIndexStartOffset + m_meshlet.triangleOffset + GetTriangleID()) + m_meshlet.vertexOffset + m_gpuMesh.vertexStartOffset;
    }
    
    uint GetObjectID()
    {
        return m_meshlet.objectId;
    }
    
    const VertexPositionData GetVertexPositionData()
    {
        return m_gpuMesh.vertexPositionsBuffer.Load(m_index);
    }
    
    const VertexMaterialData GetVertexMaterialData()
    {
        return m_gpuMesh.vertexMaterialBuffer.Load(m_index);
    }

    const float4x4 GetTransform()
    {
        return m_objectDrawData.transform;
    }

    const float4x4 GetSkinningMatrix()
    {
        [branch]
        if (!m_objectDrawData.isAnimated)
        {
            return IDENTITY_MATRIX;
        }

        VertexAnimationInfo animData = m_gpuMesh.vertexAnimationInfoBuffer.Load(m_index);        
        float4x4 result = 0.f;
        
        const uint influenceCount = animData.influenceCount;
        const uint boneOffset = animData.boneOffset;
        
        for (uint i = 0; i < influenceCount; i++)
        {
            const uint16_t influence = m_gpuMesh.vertexBoneInfluencesBuffer.Load(boneOffset + i);    
            const float weight = m_gpuMesh.vertexBoneWeightsBuffer.Load(boneOffset + i);
            result += mul(m_constants.gpuScene.bonesBuffer.Load(m_objectDrawData.boneOffset + influence), weight);
        }
        
        return result;            
    }
    
    const float3 GetNormal()
    {
        const VertexMaterialData materialData = GetVertexMaterialData();
        
        uint2 octIntNormal;
        
        octIntNormal.x = (materialData.normal >> 0) & 0xFF;
        octIntNormal.y = (materialData.normal >> 8) & 0xFF;
    
        float2 octNormal = 0.f;
        octNormal.x = octIntNormal.x / 255.f;
        octNormal.y = octIntNormal.y / 255.f;

        return OctNormalDecode(octNormal);
    }
};
