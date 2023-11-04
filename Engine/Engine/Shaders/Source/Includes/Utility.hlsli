#pragma once

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
    return (dividend - 1) / divisor + 1;
}

float4x4 ToFloat4x4(float4x3 transform)
{
    return float4x4(float4(transform[0], 0.f), float4(transform[1], 0.f), float4(transform[2], 0.f), float4(0.f, 0.f, 0.f, 1.f));
}