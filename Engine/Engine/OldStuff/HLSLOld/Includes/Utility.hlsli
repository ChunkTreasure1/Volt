#include "Buffers.hlsli"

#ifndef UTILITY_H
#define UTILITY_H

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

// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
float2 OctWrap(float2 v)
{
    float multiplier = -1.f;
    if (v.x >= 0.f && v.y >= 0.f)
    {
        multiplier = -1.f;
    }
    
    return (1.0 - abs(v.yx)) * multiplier;
}
 
float2 EncodeUnitVector(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    
    if (n.z < 0.f)
    {
        n.xy = OctWrap(n.xy);
    }
    
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}
 
float3 DecodeUnitVector(float2 f)
{
    f = f * 2.0 - 1.0;
 
// https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    
    if (n.x >= 0.f && n.y >= 0.f)
    {
        n.xy += -t;
    }
    else
    {
        n.xy += t;
    }
    
    return normalize(n);
}

float3 GetGradient(float value)
{
    float3 zero = float3(0.0, 0.0, 0.0);
    float3 white = float3(0.0, 0.1, 0.9);
    float3 red = float3(0.2, 0.9, 0.4);
    float3 blue = float3(0.8, 0.8, 0.3);
    float3 green = float3(0.9, 0.2, 0.3);

    float step0 = 0.0f;
    float step1 = 2.0f;
    float step2 = 8.0f;
    float step3 = 32.0f;
    float step4 = 64.0f;

    float3 color = lerp(zero, white, smoothstep(step0, step1, value));
    color = lerp(color, white, smoothstep(step1, step2, value));
    color = lerp(color, red, smoothstep(step1, step2, value));
    color = lerp(color, blue, smoothstep(step2, step3, value));
    color = lerp(color, green, smoothstep(step3, step4, value));

    return color;
}

float3 ReconstructWorldPositionViewProj(float4x4 proj, float4x4 view, float2 texCoords, float pixelDepth)
{
    float x = texCoords.x * 2.f - 1.f;
    float y = texCoords.y * 2.f - 1.f;

    const float4 projSpacePos = float4(x, y, pixelDepth, 1.f);
    float4 viewSpacePos = mul(proj, projSpacePos);

    viewSpacePos /= viewSpacePos.w;

    const float4 worldSpacePos = mul(view, viewSpacePos);
    return worldSpacePos.xyz;
}

float3 ReconstructWorldPosition(float2 texCoords, float pixelDepth)
{
    return ReconstructWorldPositionViewProj(u_cameraData.inverseProjection, u_cameraData.inverseView, texCoords, pixelDepth);
}

float3 ReconstructViewPosition(float2 texCoords, float pixelDepth)
{
    float x = texCoords.x * 2.f - 1.f;
    float y = texCoords.y * 2.f - 1.f;

    const float4 projSpacePos = float4(x, y, pixelDepth, 1.f);
    float4 viewSpacePos = mul(u_cameraData.inverseProjection, projSpacePos);

    viewSpacePos /= viewSpacePos.w;
    return viewSpacePos.xyz;
}

float3x3 CalculateTBN(in float3 inNormal, in float3 inTangent)
{
    const float3 normal = normalize(inNormal);
    const float3 tangent = normalize(inTangent);
    const float3 bitangent = normalize(cross(normal, tangent));

    return transpose(float3x3(tangent, bitangent, normal));
}

// Gives a linear depth value between near plane and far plane
float LinearizeDepth(const float screenDepth)
{
    float depthLinearizeMul = u_cameraData.depthUnpackConsts.x;
    float depthLinearizeAdd = u_cameraData.depthUnpackConsts.y;
    // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
    return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
}

// Gives a linear depth value between near plane and far plane
float LinearizeDepth01(const float screenDepth)
{
    float depthLinearizeMul = u_cameraData.depthUnpackConsts.x;
    float depthLinearizeAdd = u_cameraData.depthUnpackConsts.y;
    // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
    const float linearDepth = depthLinearizeMul / (depthLinearizeAdd - screenDepth);

    return (linearDepth - u_cameraData.farPlane) / (u_cameraData.nearPlane - linearDepth);
}

template<typename T>
T ClipToUV(in T clipCoords)
{
    clipCoords.xy = clipCoords.xy * 0.5f + 0.5f;
    return clipCoords;
}

#endif