#pragma once

#include "BoundingVolumes.hlsli"

namespace CullingMode
{
    static const uint Perspective = 0;
    static const uint Orthographic = 1;
    static const uint None = 2;
}

struct PerspectiveFrustumInfo
{
    float frustum0;
    float frustum1;
    float frustum2;
    float frustum3;

    float nearPlane;
    float farPlane;

    float4x4 viewMatrix;
};

bool IsInFrustum_Perspective(in PerspectiveFrustumInfo info, in BoundingSphere sphere)
{
    const float3 center = mul(info.viewMatrix, float4(sphere.center, 1.f)).xyz;

    bool visible = true;
    
    visible = visible && center.z * info.frustum1 - abs(center.x) * info.frustum0 > -sphere.radius;
    visible = visible && center.z * info.frustum3 - abs(center.y) * info.frustum2 > -sphere.radius;
    visible = visible && center.z + sphere.radius > info.nearPlane && center.z - sphere.radius < info.farPlane;
    
    return visible;
}

bool IsInFrustum_Orthographic(in BoundingBox bb, in BoundingSphere sphere)
{
    return bb.Intersects(sphere);
}