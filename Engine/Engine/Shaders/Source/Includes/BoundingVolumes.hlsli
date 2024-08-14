#pragma once

#include "Ray.hlsli"

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

struct IntersectionResult
{
    float t;
    bool hit;
};

struct BoundingBox
{
    float SquaredDistPointAABB(float3 p, float3 imin, float3 imax)
    {
        float sqDist = 0.f;
        
        [unroll]
        for (int i = 0; i < 3; ++i)
        {
            float v = p[i];
            if (v < imin[i]) sqDist += (imin[i] - v) * (imin[i] - v);
            if (v > imax[i]) sqDist += (v - imax[i]) * (v - imax[i]);
        }

        return sqDist;
    }

    bool Intersects(in BoundingSphere sphere)
    {
        const float sqDist = SquaredDistPointAABB(sphere.center, bmin, bmax);
        return sqDist <= sphere.radius * sphere.radius;
    }

    bool Intersects(in Ray ray, out float2 hitTimes)
    {
        float3 invDir = 1.f / ray.direction;

        float t1 = (bmin[0] - ray.origin[0]) * invDir[0];
        float t2 = (bmax[0] - ray.origin[0]) * invDir[0];
    
        float tmin = min(t1, t2);
        float tmax = max(t1, t2);

        for (int i = 0; i < 3; ++i)
        {
            t1 = (bmin[i] - ray.origin[i]) * invDir[i];
            t2 = (bmax[i] - ray.origin[i]) * invDir[i];

            tmin = max(tmin, min(min(t1, t2), tmax));
            tmax = min(tmax, max(max(t1, t2), tmin));
        }
         
        hitTimes = float2(tmin, tmax);
        return tmax > max(tmin, 0.f);
    } 

    float3 bmax;
    float3 bmin;

};