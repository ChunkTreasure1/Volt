#include "Defines.hlsli"
#include "Structures.hlsli"

#ifndef VERTEX_H
#define VERTEX_H

///// Vertex Buffers /////
ByteAddressBuffer u_vertexPositionsBuffers[] : register(t0, space1);
//ByteAddressBuffer u_vertexMaterialDataBuffers[] : register(t0, space2);
//ByteAddressBuffer u_vertexAniamtionDataBuffers[] : register(t0, space3);
ByteAddressBuffer u_indexBuffers[] : register(t0, space4);
//////////////////////////

///// Draw buffers /////
StructuredBuffer<uint> u_drawToInstanceOffset : register(t0, space0);
StructuredBuffer<IndirectDrawData> u_indirectDrawData : register(t1, space0);
StructuredBuffer<uint> u_instanceOffsetToObjectID : register(t2, space0);
////////////////////////

struct DefaultInput
{
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
    BUILTIN_VARIABLE("DrawIndex", uint, drawIndex);
    
    uint GetObjectID()
    {
        const uint instanceOffset = u_drawToInstanceOffset[drawIndex];
        return u_instanceOffsetToObjectID[instanceOffset + instanceId];
    }
    
    const IndirectDrawData GetDrawData()
    {
        const uint objectId = GetObjectID();
        return u_indirectDrawData[objectId];
    }
    
    const uint GetVertexIndex()
    {
        const IndirectDrawData drawData = GetDrawData();
        
        const uint vertexIndex = u_indexBuffers[NonUniformResourceIndex(drawData.meshId)].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        return vertexIndex;
    }
    
    const float3 GetLocalPosition()
    {
        const IndirectDrawData drawData = GetDrawData();
        
        const uint vertexIndex = u_indexBuffers[NonUniformResourceIndex(drawData.meshId)].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        const float3 position = u_vertexPositionsBuffers[NonUniformResourceIndex(drawData.meshId)].Load<float3>(sizeof(float3) * vertexIndex);
        
        return position;
    }
    
    const float4 GetWorldPosition()
    {
        const IndirectDrawData drawData = GetDrawData();
        
        const uint vertexIndex = u_indexBuffers[NonUniformResourceIndex(drawData.meshId)].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        const float3 position = u_vertexPositionsBuffers[NonUniformResourceIndex(drawData.meshId)].Load<float3>(sizeof(float3) * vertexIndex);
        
        return mul(drawData.transform, float4(position, 1.f));
    }
    
    const float4x4 GetTransform()
    {
        const IndirectDrawData drawData = GetDrawData();
        return drawData.transform;
    }
};

#endif