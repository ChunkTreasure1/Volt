#include "DrawBuffers.hlsli"
#include "Bindless.hlsli"

#ifndef VERTEX_H
#define VERTEX_H

static const uint VERTEX_MATERIAL_DATA_SIZE = 12;
static const uint VERTEX_ANIMATION_DATA_SIZE = 16;

float2 OctNormalWrap(float2 v)
{
    float2 wrap;
    wrap.x = (1.0f - abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
    wrap.y = (1.0f - abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
    return wrap;
}

float2 OctNormalEncode(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));

    float2 wrapped = OctNormalWrap(n.xy);

    float2 result;
    result.x = n.z >= 0.0f ? n.x : wrapped.x;
    result.y = n.z >= 0.0f ? n.y : wrapped.y;

    result.x = result.x * 0.5f + 0.5f;
    result.y = result.y * 0.5f + 0.5f;

    return result;
}

// From https://www.jeremyong.com/graphics/2023/01/09/tangent-spaces-and-diamond-encoding/
float DiamondEncode(float2 p)
{
			// Project to the unit diamond, then to the x-axis.
    float x = p.x / (abs(p.x) + abs(p.y));
		
			// Contract the x coordinate by a factor of 4 to represent all 4 quadrants in
			// the unit range and remap
    float pySign = 0.f;
    if (p.y < 0.f)
    {
        pySign = -1.f;
    }
    else if (p.y > 0.f)
    {
        pySign = 1.f;
    }

    return -pySign * 0.25f * x + 0.5f + pySign * 0.25f;
}

// Given a normal and tangent vector, encode the tangent as a single float that can be
// subsequently quantized.
float EncodeTangent(float3 normal, float3 tangent)
{
			// First, find a canonical direction in the tangent plane
    float3 t1;
    if (abs(normal.y) > abs(normal.z))
    {
				// Pick a canonical direction orthogonal to n with z = 0
        t1 = float3(normal.y, -normal.x, 0.f);
    }
    else
    {
				// Pick a canonical direction orthogonal to n with y = 0
        t1 = float3(normal.z, 0.f, -normal.x);
    }
    t1 = normalize(t1);

			// Construct t2 such that t1 and t2 span the plane
    float3 t2 = cross(t1, normal);

			// Decompose the tangent into two coordinates in the canonical basis
    float2 packed_tangent = float2(dot(tangent, t1), dot(tangent, t2));

			// Apply our diamond encoding to our two coordinates
    return DiamondEncode(packed_tangent);
}

float4 UnpackUIntToFloat4(uint packedValue)
{
    float4 result = 0.f;
    
    // Extract the components from the packed uint
    uint packedX = (packedValue >> 24) & 0xFF;
    uint packedY = (packedValue >> 16) & 0xFF;
    uint packedZ = (packedValue >> 8) & 0xFF;
    uint packedW = packedValue & 0xFF;

    // Convert packed components back to float range
    result.x = float(packedX) / 255.0f;
    result.y = float(packedY) / 255.0f;
    result.z = float(packedZ) / 255.0f;
    result.w = float(packedW) / 255.0f;
    
    return result;
}

float3 OctNormalDecode(float2 f)
{
    f = f * 2.0 - 1.0;
 
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0.f, 1.f);
	
    n.x += n.x >= 0.0f ? -t : t;
    n.y += n.y >= 0.0f ? -t : t;

    return normalize(n);
}

float2 decode_diamond(float p)
{
    float2 v;

    // Remap p to the appropriate segment on the diamond
    float p_sign = sign(p - 0.5f);
    v.x = -p_sign * 4.f * p + 1.f + p_sign * 2.f;
    v.y = p_sign * (1.f - abs(v.x));

    // Normalization extends the point on the diamond back to the unit circle
    return normalize(v);
}

float3 decode_tangent(float3 normal, float diamond_tangent)
{
    // As in the encode step, find our canonical tangent basis span(t1, t2)
    float3 t1;
    if (abs(normal.y) > abs(normal.z))
    {
        t1 = float3(normal.y, -normal.x, 0.f);
    }
    else
    {
        t1 = float3(normal.z, 0.f, -normal.x);
    }
    t1 = normalize(t1);

    float3 t2 = cross(t1, normal);

    // Recover the coordinates used with t1 and t2
    float2 packed_tangent = decode_diamond(diamond_tangent);

    return packed_tangent.x * t1 + packed_tangent.y * t2;
}

struct DefaultInput
{
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
    BUILTIN_VARIABLE("DrawIndex", uint, drawIndex);
    
    uint GetObjectID()
    {
        const uint instanceOffset = u_drawToInstanceOffset[drawIndex];
        return instanceOffset;
        //return u_instanceOffsetToObjectID[instanceOffset + instanceId];
    }
    
    const IndirectDrawData GetDrawData()
    {
        const uint objectId = GetObjectID();
        return u_indirectDrawData[objectId];
    }
    
    const uint GetTriangleID()
    {
        return vertexId / 3;
    }
    
    const uint GetVertexIndex()
    {
        const IndirectDrawData drawData = GetDrawData();
        
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        return vertexIndex;
    }
    
    const float3 GetLocalPosition()
    {
        const IndirectDrawData drawData = GetDrawData();
        
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        const float3 position = u_vertexPositionsBuffers[drawData.meshId].Load<float3>(sizeof(float3) * vertexIndex);
        
        return position;
    }
    
    const float4 GetWorldPosition()
    {
        const IndirectDrawData drawData = GetDrawData();
        
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        const float3 position = u_vertexPositionsBuffers[drawData.meshId].Load<float3>(sizeof(float3) * vertexIndex);
        
        return mul(drawData.transform, float4(position, 1.f));
    }
    
    const float3 GetNormal()
    {
        const IndirectDrawData drawData = GetDrawData();
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    
        uint normalValues = u_vertexMaterialDataBuffers[drawData.meshId].Load(VERTEX_MATERIAL_DATA_SIZE * vertexIndex);
        
        uint2 octIntNormal;
        
        octIntNormal.x = (normalValues >> 8) & 0xFF;
        octIntNormal.y = (normalValues >> 0) & 0xFF;
    
        float2 octNormal = 0.f;
        octNormal.x = octIntNormal.x / 255.f;
        octNormal.y = octIntNormal.y / 255.f;

        return OctNormalDecode(octNormal);
    }
    
    const float3 GetTangent()
    {    
        const IndirectDrawData drawData = GetDrawData();
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    
        float tangent = u_vertexMaterialDataBuffers[drawData.meshId].Load<float>((VERTEX_MATERIAL_DATA_SIZE * vertexIndex) + 4);
        
        return decode_tangent(GetNormal(), tangent);
    }
    
    const float2 GetTexCoords()
    {
        const IndirectDrawData drawData = GetDrawData();
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
        const uint texCoordsUINT = u_vertexMaterialDataBuffers[drawData.meshId].Load<uint>((VERTEX_MATERIAL_DATA_SIZE * vertexIndex) + 8);
        
        float2 result;
        result.x = asfloat((texCoordsUINT >> 16) & 0xFFFF);
        result.y = asfloat((texCoordsUINT >> 0) & 0xFFFF);
    
        return result;
    }
    
    float4x4 GetSkinnedMatrix()
    {
        const IndirectDrawData drawData = GetDrawData();
        const uint vertexIndex = u_indexBuffers[drawData.meshId].Load<uint>(sizeof(uint) * vertexId) + drawData.vertexStartOffset;
    
        uint4 influences;
        
        uint influenceXY = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 0));
        influences.x = (influenceXY >> 16) & 0xFFFF;
        influences.y = (influenceXY >> 0) & 0xFFFF;
        
        uint influenceZW = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 4));
        influences.z = (influenceZW >> 16) & 0xFFFF;
        influences.w = (influenceZW >> 0) & 0xFFFF;
    
        float4 weights;
        
        uint weightsXY = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 8));
        weights.x = asfloat((weightsXY >> 16) & 0xFFFF);
        weights.y = asfloat((weightsXY >> 0) & 0xFFFF);
        
        uint weightsZW = u_vertexAnimationDataBuffers[drawData.meshId].Load<uint>((VERTEX_ANIMATION_DATA_SIZE * vertexIndex + 12));
        weights.z = asfloat((weightsZW >> 16) & 0xFFFF);
        weights.w = asfloat((weightsZW >> 0) & 0xFFFF);
        
        float4x4 skinningMatrix = 0.f;
        //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.x], float(weights.x));
        //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.y], float(weights.y));
        //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.z], float(weights.z));
        //skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.w], float(weights.w));

        return skinningMatrix;
    }
    
    const float4x4 GetTransform()
    {
        const IndirectDrawData drawData = GetDrawData();
        return drawData.transform;
    }
};

#endif