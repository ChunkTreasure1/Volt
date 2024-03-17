#pragma once

#include "Structures.hlsli"

float3 GetRandomColor(uint _seed)
{
    uint hash = (_seed ^ 61) ^ (_seed >> 16);
    hash = hash + (hash << 3);
    hash = hash ^ (hash >> 4);
    hash = hash * 0x27d4eb2d;
    hash = hash ^ (hash >> 15);
    return float3(float(hash & 255),
                float((hash >> 8) & 255),
                float((hash >> 16) & 255)) / 255.0;
}

uint DivideRoundUp(uint dividend, uint divisor)
{
    return (dividend + divisor - 1) / divisor;
}

float4x4 ToFloat4x4(float4x3 transform)
{
    return float4x4(float4(transform[0], 0.f), float4(transform[1], 0.f), float4(transform[2], 0.f), float4(0.f, 0.f, 0.f, 1.f));
}

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

// Gives a linear depth value between near plane and far plane
float LinearizeDepth(const float screenDepth, in const ViewData viewData)
{
    float depthLinearizeMul = viewData.depthUnpackConsts.x;
    float depthLinearizeAdd = viewData.depthUnpackConsts.y;
    // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
    return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
}

// Gives a linear depth value between near plane and far plane in range [0...1]
float LinearizeDepth01(const float screenDepth, in const ViewData viewData)
{
    const float linearDepth = LinearizeDepth(screenDepth, viewData);
    return (linearDepth - viewData.nearPlane) / (viewData.farPlane - viewData.nearPlane);
}

float3x3 CalculateTBN(float3 inNormal, float3 inTangent)
{
    const float3 normal = normalize(inNormal);
    const float3 tangent = normalize(inTangent);
    const float3 bitangent = normalize(cross(normal, tangent));
    
    return transpose(float3x3(tangent, bitangent, normal));
}

float3 ReconstructWorldPosition(in ViewData viewData, float2 texCoords, float pixelDepth)
{
    float x = texCoords.x * 2.f - 1.f;
    float y = texCoords.y * 2.f - 1.f;
    
    const float4 projSpacePos = float4(x, y, pixelDepth, 1.f);
    float4 viewSpacePos = mul(viewData.inverseProjection, projSpacePos);
    
    viewSpacePos /= viewSpacePos.w;
    
    const float4 worldSpacePos = mul(viewData.inverseView, viewSpacePos);
    return worldSpacePos.xyz;
}