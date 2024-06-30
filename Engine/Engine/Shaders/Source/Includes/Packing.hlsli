#pragma once

#include "Utility.hlsli"

const float3 DecodeNormal(uint normalInt)
{
    uint2 octIntNormal;
    
    octIntNormal.x = (normalInt >> 0) & 0xFF;
    octIntNormal.y = (normalInt >> 8) & 0xFF;
    
    float2 octNormal = 0.f;
    octNormal.x = octIntNormal.x / 255.f;
    octNormal.y = octIntNormal.y / 255.f;
    
    return OctNormalDecode(octNormal);
}

const float3 DecodeTangent(float3 normal, float tangentFloat)
{
    return decode_tangent(normal, tangentFloat);
}