#pragma once

struct Transform
{
    float4 rotation;
    float3 position;
    float3 scale;

    float3 RotateVector(float3 v)
    {
        return v + 2.f * cross(rotation.xyz, cross(rotation.xyz, v) + rotation.w * v);
    }

    float3 GetWorldPosition(float3 vertexPos)
    {
        return RotateVector(vertexPos * scale) + position;
    }
};