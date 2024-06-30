#pragma once

#include "Defines.hlsli"
#include "GPUScene.hlsli"
#include "DrawContext.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"

static const uint VERTEX_MATERIAL_DATA_SIZE = 12;
static const uint VERTEX_ANIMATION_DATA_SIZE = 16;

struct Constants
{
    UniformBuffer<GPUScene> gpuScene;
    TypedBuffer<DrawContext> drawContext;
    
    TypedBuffer<CameraData> cameraData; // #TODO_Ivar: Should be uniform buffer
};

struct DefaultInput
{
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
    BUILTIN_VARIABLE("DrawIndex", uint, drawIndex);
    
    uint GetObjectID()
    {
        const Constants constants = GetConstants<Constants>();
        const DrawContext context = constants.drawContext.Load(0);
   
        const uint objectId = context.drawToInstanceOffset.Load(drawIndex);
        return objectId;
        //return u_instanceOffsetToObjectID[instanceOffset + instanceId];
    }
    
    //const ObjectDrawData GetDrawData()
    //{
    //    const uint objectId = GetObjectID();
        
    //    const Constants constants = GetConstants<Constants>();
    //    const GPUScene scene = constants.gpuScene.Load(0);
        
    //    return scene.objectDrawDataBuffer.Load(objectId);
    //}
    
    const uint GetTriangleID()
    {
        return vertexId / 3;
    }
    
    const VertexPositionData GetVertexPositionData()
    {
        const Constants constants = GetConstants<Constants>();
        const GPUScene scene = constants.gpuScene.Load();
        const DrawContext context = constants.drawContext.Load(0);
   
        const uint objectId = context.drawToInstanceOffset.Load(drawIndex);
        const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
        const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    
        const uint vertexIndex = mesh.indexBuffer.Load(vertexId) + mesh.vertexStartOffset;
        return mesh.vertexPositionsBuffer.Load(vertexIndex);
    }
    
    const VertexMaterialData GetVertexMaterialData()
    {
        const Constants constants = GetConstants < Constants > ();
        const GPUScene scene = constants.gpuScene.Load();
        const DrawContext context = constants.drawContext.Load(0);
   
        const uint objectId = context.drawToInstanceOffset.Load(drawIndex);
        const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
        const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    
        const uint vertexIndex = mesh.indexBuffer.Load(vertexId) + mesh.vertexStartOffset;
        return mesh.vertexMaterialBuffer.Load(vertexIndex);
    }

    const float4x4 GetTransform()
    {
        const Constants constants = GetConstants<Constants>();
        const GPUScene scene = constants.gpuScene.Load();
        const DrawContext context = constants.drawContext.Load(0);
   
        const uint objectId = context.drawToInstanceOffset.Load(drawIndex);
        const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
        
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
    
    //const float3 GetNormal()
    //{
    //    const IndirectDrawData drawData = GetDrawData();
    //    const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    
    //    uint normalValues = u_vertexMaterialDataBuffers[drawData.meshId].Load(VERTEX_MATERIAL_DATA_SIZE * vertexIndex);
        
    //    uint2 octIntNormal;
        
    //    octIntNormal.x = (normalValues >> 8) & 0xFF;
    //    octIntNormal.y = (normalValues >> 0) & 0xFF;
    
    //    float2 octNormal = 0.f;
    //    octNormal.x = octIntNormal.x / 255.f;
    //    octNormal.y = octIntNormal.y / 255.f;

    //    return OctNormalDecode(octNormal);
    //}
    
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
