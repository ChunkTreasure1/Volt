#include <Common.hlsli>

#ifndef FOGCOMMON_H
#define FOGCOMMON_H

#define VOXEL_GRID_SIZE_X 160
#define VOXEL_GRID_SIZE_Y 90
#define VOXEL_GRID_SIZE_Z 92

float LinearizeDepth(float depth, float nearPlane, float farPlane)
{
    float y = farPlane / nearPlane;
    float x = 1.f - y;
    
    return 1.f / (x * depth + y);
}

float DelinearizeDepth(float depth, float nearPlane, float farPlane)
{
    float y = farPlane / nearPlane;
    float x = 1.f - y;
    
    return (1.f / depth - y) / x;
}

float3 GetNDCCoordsFromTexCoords(float3 texCoords, float nearPlane, float farPlane)
{
    float3 ndc;
    ndc.x = 2.f * texCoords.x - 1.f;
    ndc.y = 2.f * texCoords.y - 1.f;
    ndc.z = 2.f * DelinearizeDepth(texCoords.z, nearPlane, farPlane) - 1.f;
    
    return ndc;
}

float3 GetTexCoordsFromIndex_Jitter(uint3 index, float nearPlane, float farPlane, float jitter)
{
    const float viewZ = nearPlane * pow(farPlane / nearPlane, (float(index.z) + 0.5f + jitter) / float(VOXEL_GRID_SIZE_Z));

    return float3((float(index.x) + 0.5f) / float(VOXEL_GRID_SIZE_X),
                  (float(index.y) + 0.5f) / float(VOXEL_GRID_SIZE_Y),
                  viewZ / farPlane);
}

float3 GetWorldPositionFromNDCCoords(in float3 ndcCoords, in float4x4 invesrseViewProj)
{
    float4 position = mul(invesrseViewProj, float4(ndcCoords, 1.f));

    position.xyz /= position.w;
    return position.xyz;
}

float3 GetNDCCoordsFromWorldPosition(in float3 worldPos, in float4x4 viewProjection)
{
    float4 position = mul(viewProjection, float4(worldPos, 1.f));

    if (position.w > 0.f)
    {
        position.xyz /= position.w;
    }
    
    return position.xyz;
}

float3 GetTexCoordsFromNDCCoords(in float3 ndc, float nearPlane, float farPlane)
{
    float3 texCoords;
    
    texCoords.x = ndc.x * 0.5f + 0.5f;
    texCoords.y = ndc.y * 0.5f + 0.5f;
    texCoords.z = LinearizeDepth(ndc.z * 0.5f + 0.5f, nearPlane, farPlane);
    
    const float2 params = float2(float(VOXEL_GRID_SIZE_Z) / log2(farPlane / nearPlane), -(float(VOXEL_GRID_SIZE_Z) * log2(nearPlane) / log2(farPlane / nearPlane)));
    float viewZ = texCoords.z * farPlane;
    texCoords.z = (max(log2(viewZ) * params.x + params.y, 0.f)) / VOXEL_GRID_SIZE_Z;

    return texCoords;
}

float3 GetTexCoordsFromWorldPosition(in float3 worldPos, float nearPlane, float farPlane, in float4x4 viewProjection)
{
    const float3 ndc = GetNDCCoordsFromWorldPosition(worldPos, viewProjection);
    return GetTexCoordsFromNDCCoords(ndc, nearPlane, farPlane);
}

float3 GetWorldPositionFromIndex_Jitter(uint3 index, float jitter, float nearPlane, float farPlane, in float4x4 inverseViewProj)
{
    const float3 texCoords = GetTexCoordsFromIndex_Jitter(index, nearPlane, farPlane, jitter);
    const float3 ndcCoords = GetNDCCoordsFromTexCoords(texCoords, nearPlane, farPlane);
    return GetWorldPositionFromNDCCoords(ndcCoords, inverseViewProj);
}

// https://gist.github.com/Fewes/59d2c831672040452aa77da6eaab2234
//! Tricubic interpolated texture lookup, using unnormalized coordinates.
//! Fast implementation, using 8 trilinear lookups.
//! @param tex  3D texture
//! @param coord  normalized 3D texture coordinate
//! @param textureSize  the size (resolution) of the texture
float4 tex3DTricubic(Texture3D tex, SamplerState state, float3 coord, float3 textureSize)
{
    // Shift the coordinate from [0,1] to [-0.5, textureSize-0.5]
    float3 coord_grid = coord * textureSize - 0.5;
    float3 index = floor(coord_grid);
    float3 fraction = coord_grid - index;
    float3 one_frac = 1.0 - fraction;

    float3 w0 = 1.0 / 6.0 * one_frac * one_frac * one_frac;
    float3 w1 = 2.0 / 3.0 - 0.5 * fraction * fraction * (2.0 - fraction);
    float3 w2 = 2.0 / 3.0 - 0.5 * one_frac * one_frac * (2.0 - one_frac);
    float3 w3 = 1.0 / 6.0 * fraction * fraction * fraction;

    float3 g0 = w0 + w1;
    float3 g1 = w2 + w3;
    float3 mult = 1.0 / textureSize;
    float3 h0 = mult * ((w1 / g0) - 0.5 + index); //h0 = w1/g0 - 1, move from [-0.5, textureSize-0.5] to [0,1]
    float3 h1 = mult * ((w3 / g1) + 1.5 + index); //h1 = w3/g1 + 1, move from [-0.5, textureSize-0.5] to [0,1]

    // Fetch the eight linear interpolations
    // Weighting and fetching is interleaved for performance and stability reasons
    float4 tex000 = tex.SampleLevel(state, float3(h0), 0.f);
    float4 tex100 = tex.SampleLevel(state, float3(h1.x, h0.y, h0.z), 0.f);
    tex000 = lerp(tex100, tex000, g0.x); // Weigh along the x-direction

    float4 tex010 = tex.SampleLevel(state, float3(h0.x, h1.y, h0.z), 0.f);
    float4 tex110 = tex.SampleLevel(state, float3(h1.x, h1.y, h0.z), 0.f);
    tex010 = lerp(tex110, tex010, g0.x); // Weigh along the x-direction
    tex000 = lerp(tex010, tex000, g0.y); // Weigh along the y-direction

    float4 tex001 = tex.SampleLevel(state, float3(h0.x, h0.y, h1.z), 0.f);
    float4 tex101 = tex.SampleLevel(state, float3(h1.x, h0.y, h1.z), 0.f);
    tex001 = lerp(tex101, tex001, g0.x); // Weigh along the x-direction

    float4 tex011 = tex.SampleLevel(state, float3(h0.x, h1.y, h1.z), 0.f);
    float4 tex111 = tex.SampleLevel(state, float3(h1), 0.f);
    tex011 = lerp(tex111, tex011, g0.x); // Weigh along the x-direction
    tex001 = lerp(tex011, tex001, g0.y); // Weigh along the y-direction

    return lerp(tex001, tex000, g0.z); // Weigh along the z-direction
}
#endif