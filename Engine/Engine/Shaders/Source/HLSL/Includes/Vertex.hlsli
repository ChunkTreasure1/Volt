#include "Buffers.hlsli"

#ifndef VERTEX_H
#define VERTEX_H

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

struct DefaultVertexInput
{
    STAGE_VARIABLE(float3, position, POSITION, 0);
    
    STAGE_VARIABLE(uint4, normal, NORMAL, 1);
    STAGE_VARIABLE(float, tangent, TANGENT, 2);
    
    STAGE_VARIABLE(half2, texCoords, TEXCOORDS, 3);
    
    STAGE_VARIABLE(uint4, influences, INFLUENCES, 4);
    STAGE_VARIABLE(half4, weights, WEIGHTS, 5);
    
    uint vertexId : SV_VertexID;
    
    BUILTIN_VARIABLE("BaseInstance", uint, baseInstance);
    BUILTIN_VARIABLE("DrawIndex", uint, drawIndex);
    
    ObjectData GetObjectData()
    {
        const uint objectMapIndex = u_drawToObjectId[drawIndex + baseInstance];
        return u_objectData[objectMapIndex];
    }

    uint GetObjectIndex()
    {
        const uint objectMapIndex = u_drawToObjectId[drawIndex + baseInstance];
        return objectMapIndex;
    }
    
    float4x4 GetSkinnedMatrix()
    {
        const uint objectMapIndex = u_drawToObjectId[drawIndex + baseInstance];
        const ObjectData objectData = u_objectData[objectMapIndex];
    
        float4x4 skinningMatrix = 0.f;
        skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.x], float(weights.x));
        skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.y], float(weights.y));
        skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.z], float(weights.z));
        skinningMatrix += mul(u_animationData[objectData.boneOffset + influences.w], float(weights.w));

        return skinningMatrix;
    }
    
    float3 GetPosition()
    {
        return position;
    }
    
    float3 GetNormal()
    {
        float2 octNormal = 0.f;
        octNormal.x = normal.x / 255.f;
        octNormal.y = normal.y / 255.f;
    
        return OctNormalDecode(octNormal);
    }
    
    float3 GetTangent()
    {
        return decode_tangent(GetNormal(), tangent);
    }
    
    float4 GetPaintedColor()
    {
        const ObjectData objectData = GetObjectData();
        
        if (objectData.colorOffset == -1)
        {
            return 0.f;
        }
        
        return u_paintedVertexColors[objectData.colorOffset + vertexId];
    }
    
    //float3 GetColor()
    //{
    //    return float3(color.x / 255.f, color.y / 255.f, color.z / 255.f);
    //}
    
    float2 GetTexCoords()
    {
        return float2(texCoords.x, texCoords.y);
    }
};

struct DefaultPixelInput
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float3, normal, NORMAL, 0);
    STAGE_VARIABLE(float3, tangent, TANGENT, 1);
    STAGE_VARIABLE(float3, worldPosition, WORLDPOSITION, 2);
    STAGE_VARIABLE(float2, texCoords, TEXCOORD, 3);
    STAGE_VARIABLE(uint, materialIndex, MATINDEX, 4);
    STAGE_VARIABLE(uint, objectIndex, OBJINDEX, 5);
    STAGE_VARIABLE(float4, paintedColor, PAINTEDCOLOR, 6);
    
    ObjectData GetObjectData()
    {
        const uint objectMapIndex = u_drawToObjectId[objectIndex];
        return u_objectData[objectMapIndex];
    }
};

struct DefaultFullscreenTriangleVertex
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float2, texCoords, TEXCOORD, 0);
};

struct DefaultQuadVertex
{
    STAGE_VARIABLE(float4, position, POSITION, 0);
    STAGE_VARIABLE(float4, color, COLOR, 1);
    STAGE_VARIABLE(float2, texCoords, TEXCOORDS, 2);
};

struct DefaultParticleVertex
{
    uint instanceId : INSTANCEID;
};

struct DefaultBillboardVertex
{
    STAGE_VARIABLE(float4, position, POSITION, 0);
    STAGE_VARIABLE(float4, color, COLOR, 1);
    STAGE_VARIABLE(float3, scale, SCALE, 2);
    STAGE_VARIABLE(uint, textureIndex, TEXINDEX, 3);
    STAGE_VARIABLE(uint, id, ID, 4);
};

struct DefaultTextVertex
{
    STAGE_VARIABLE(float4, position, POSITION, 0);
    STAGE_VARIABLE(float4, color, COLOR, 1);
    STAGE_VARIABLE(float2, texCoords, TEXCOORDS, 2);
    STAGE_VARIABLE(uint, textureIndex, TEXINDEX, 3);
};

#endif