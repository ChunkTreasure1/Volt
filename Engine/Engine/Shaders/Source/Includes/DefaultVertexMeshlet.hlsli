#pragma once

#include "Defines.hlsli"
#include "GPUScene.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"
#include "MeshletHelpers.hlsli"

static const uint VERTEX_MATERIAL_DATA_SIZE = 12;
static const uint VERTEX_ANIMATION_DATA_SIZE = 16;

struct Constants
{
    TypedBuffer<GPUScene> gpuScene;
    TypedBuffer<ViewData> viewData; // #TODO_Ivar: Should be uniform buffer
};

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
    
    uint GetObjectID()
    {
        const Constants constants = GetConstants<Constants>();
        const GPUScene scene = constants.gpuScene.Load(0);
        const Meshlet meshlet = scene.meshletsBuffer.Load(GetMeshletID());
   
        return meshlet.objectId;
    }
    
    const VertexPositionData GetVertexPositionData()
    {
        const Constants constants = GetConstants<Constants>();
        const GPUScene scene = constants.gpuScene.Load(0);
       
        const uint meshletId = GetMeshletID();
        
        const Meshlet meshlet = scene.meshletsBuffer.Load(meshletId);
        const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(meshlet.objectId);
        const GPUMesh mesh = scene.meshesBuffer.Load(meshlet.meshId);
        
        const uint index = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + GetTriangleID()) + meshlet.vertexOffset + mesh.vertexStartOffset;
        return mesh.vertexPositionsBuffer.Load(index);
    }
    
    const VertexMaterialData GetVertexMaterialData()
    {
        const Constants constants = GetConstants<Constants>();
        const GPUScene scene = constants.gpuScene.Load(0);
       
        const uint meshletId = GetMeshletID();
        
        const Meshlet meshlet = scene.meshletsBuffer.Load(meshletId);
        const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(meshlet.objectId);
        const GPUMesh mesh = scene.meshesBuffer.Load(meshlet.meshId);
        
        const uint index = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + GetTriangleID()) + meshlet.vertexOffset + mesh.vertexStartOffset;
        return mesh.vertexMaterialBuffer.Load(index);
    }

    const float4x4 GetTransform()
    {
        const Constants constants = GetConstants < Constants > ();
        const GPUScene scene = constants.gpuScene.Load(0);
       
        const uint meshletId = GetMeshletID();
        
        const Meshlet meshlet = scene.meshletsBuffer.Load(meshletId);
        const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(meshlet.objectId);
        return drawData.transform;
    }
    
    //const GPUMesh GetMesh()
    //{
    //    const Constants constants = GetConstants<Constants>();
    //    const GPUScene scene = constants.gpuScene.Load(0);
    //    const ObjectDrawData drawData = GetDrawData();
        
    //    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    //    return mesh;
    //}
    
    //const uint GetVertexIndex()
    //{
    //    const Constants constants = GetConstants<Constants>();
    //    const GPUScene scene = constants.gpuScene.Load(0);
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
