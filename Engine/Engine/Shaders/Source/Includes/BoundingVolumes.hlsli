#pragma once

struct BoundingSphere
{
    float3 center;
    float radius;

    static BoundingSphere Construct(float inRadius, float3 inCenter)
    {
        BoundingSphere result;
        result.radius = inRadius;
        result.center = inCenter;

        return result;
    }
};

struct BoundingBox
{
    float SquaredDistPointAABB(float3 p, float3 bmin, float3 bmax)
    {
        float sqDist = 0.f;
        
        [unroll]
        for (int i = 0; i < 3; ++i)
        {
            float v = p[i];
            if (v < bmin[i]) sqDist += (bmin[i] - v) * (bmin[i] - v);
            if (v > bmax[i]) sqDist += (v - bmax[i]) * (v - bmax[i]);
        }

        return sqDist;
    }

    bool Intersects(in BoundingSphere sphere)
    {
        const float sqDist = SquaredDistPointAABB(sphere.center, min, max);
        return sqDist <= sphere.radius * sphere.radius;
    }

    float3 min;
    float3 max;
};