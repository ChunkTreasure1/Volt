#pragma once

#include "Defines.hlsli"
#include "GPUScene.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"
#include "MeshletHelpers.hlsli"
#include "Matrix.hlsli"

static const uint VERTEX_MATERIAL_DATA_SIZE = 12;
static const uint VERTEX_ANIMATION_DATA_SIZE = 16;

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

        //VertexAnimationInfo animData = m_gpuMesh.vertexAnimationInfoBuffer.Load(m_index);        
        //float4x4 result = 0.f;
        //
        //const uint influenceCount = animData.influenceCount;
        //
        //for (uint i = 0; i < influenceCount; i++)
        //{
        //    const uint16_t influence = m_gpuMesh.vertexBoneInfluencesBuffer.Load(animData.boneOffset + i);    
        //    const float weight = m_gpuMesh.vertexBoneWeightsBuffer.Load(animData.boneOffset + i);
        //    result += mul(m_constants.gpuScene.bonesBuffer.Load(m_objectDrawData.boneOffset + influence), weight);
        //}
        //
        //return result;            

        VertexAnimationData animData = m_gpuMesh.vertexAnimationInfoBuffer.Load(m_index);        
        float4x4 result = 0.f;

        result += mul(m_constants.gpuScene.bonesBuffer.Load(m_objectDrawData.boneOffset + animData.influences[0]), animData.weights[0]);
        result += mul(m_constants.gpuScene.bonesBuffer.Load(m_objectDrawData.boneOffset + animData.influences[1]), animData.weights[1]);
        result += mul(m_constants.gpuScene.bonesBuffer.Load(m_objectDrawData.boneOffset + animData.influences[2]), animData.weights[2]);
        result += mul(m_constants.gpuScene.bonesBuffer.Load(m_objectDrawData.boneOffset + animData.influences[3]), animData.weights[3]);

        return result;
    }
    
    //const GPUMesh GetMesh()
    //{
    //    const Constants constants = GetConstants<Constants>();
    //    const GPUScene scene = constants.gpuScene;
    //    const ObjectDrawData drawData = GetDrawData();
        
    //    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    //    return mesh;
    //}
    
    //const uint GetVertexIndex()
    //{
    //    const Constants constants = GetConstants<Constants>();
    //    const GPUScene scene = constants.gpuScene;
    //    const ObjectDrawData drawData = GetDrawData();
        
    //    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
        
    //    const uint vertexIndex = mesh.indexBuffer.Load(vertexId) + mesh.vertexStartOffset;
    //    return vertexIndex;
    //}
    
    //const float3 GetLocalPosition()
    //{
    //    const IndirectDrawData drawData = GetDrawData();
        
    //    const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    //    const float3 position = u_vertexPositionsBuffers[drawData.meshId].Load<float3>(sizeof(float3) * vertexIndex);
        
    //    return position;
    //}
    
    //const float4 GetWorldPosition()
    //{
    //    const IndirectDrawData drawData = GetDrawData();
        
    //    const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    //    const float3 position = u_vertexPositionsBuffers[drawData.meshId].Load<float3>(sizeof(float3) * vertexIndex);
        
    //    return mul(drawData.transform, float4(position, 1.f));
    //}
    
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
    
    //const float3 GetTangent()
    //{    
    //    const IndirectDrawData drawData = GetDrawData();
    //    const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    
    //    float tangent = u_vertexMaterialDataBuffers[drawData.meshId].Load<float>((VERTEX_MATERIAL_DATA_SIZE * vertexIndex) + 4);
        
    //    return decode_tangent(GetNormal(), tangent);
    //}
    
    //const float2 GetTexCoords()
    //{
    //    const IndirectDrawData drawData = GetDrawData();
    //    const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    //    const uint texCoordsUINT = u_vertexMaterialDataBuffers[drawData.meshId].Load<uint>((VERTEX_MATERIAL_DATA_SIZE * vertexIndex) + 8);
        
    //    float2 result;
    //    result.x = asfloat((texCoordsUINT >> 16) & 0xFFFF);
    //    result.y = asfloat((texCoordsUINT >> 0) & 0xFFFF);
    
    //    return result;
    //}
    
    //float4x4 GetSkinnedMatrix()
    //{
    //    const IndirectDrawData drawData = GetDrawData();
    //    const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    
    //    uint4 influences;
        
    //    uint influenceXY = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 0));
    //    influences.x = (influenceXY >> 16) & 0xFFFF;
    //    influences.y = (influenceXY >> 0) & 0xFFFF;
        
    //    uint influenceZW = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 4));
    //    influences.z = (influenceZW >> 16) & 0xFFFF;
    //    influences.w = (influenceZW >> 0) & 0xFFFF;
    
    //    float4 weights;
        
    //    uint weightsXY = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 8));
    //    weights.x = asfloat((weightsXY >> 16) & 0xFFFF);
    //    weights.y = asfloat((weightsXY >> 0) & 0xFFFF);
        
    //    uint weightsZW = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 12));
    //    weights.z = asfloat((weightsZW >> 16) & 0xFFFF);
    //    weights.w = asfloat((weightsZW >> 0) & 0xFFFF);
        
    //    float4x4 skinningMatrix = 0.f;
    //    //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.x], float(weights.x));
    //    //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.y], float(weights.y));
    //    //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.z], float(weights.z));
    //    //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.w], float(weights.w));

    //    return skinningMatrix;
    //}
    
    //const float4x4 GetTransform()
    //{
    //    const IndirectDrawData drawData = GetDrawData();
    //    return drawData.transform;
    //}
};
