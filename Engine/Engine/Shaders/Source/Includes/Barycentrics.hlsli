#pragma once

struct BarycentricDeriv
{
    float3 lambda;
    float3 ddx;
    float3 ddy;
};

struct GradientInterpolationResults
{
    float2 interpolated;
    float2 dx;
    float2 dy;
};

float3 RayTriangleIntersection(float3 p0, float3 p1, float3 p2, float3 o, float3 d)
{
    float3 v0v1 = p1 - p0;
    float3 v0v2 = p2 - p0;
    
    float3 pvec = cross(d, v0v2);
    float det = dot(v0v1, pvec);
    float invDet = 1.f / det;
    
    float3 tvec = o - p0;
    float u = dot(tvec, pvec) * invDet;
    
    float3 qvec = cross(tvec, v0v1);
    float v = dot(d, qvec) * invDet;
    float w = 1.f - v - u;
    
    return float3(w, u, v);
}

BarycentricDeriv CalculateFullBary(float4 pt0, float4 pt1, float4 pt2, float2 pixelNdc, float2 winSize)
{
    BarycentricDeriv result;

    float3 invW = 1.f / float3(pt0.w, pt1.w, pt2.w);
    
    // Project points on screen to get post projection positions
    float2 ndc0 = pt0.xy * invW.x;
    float2 ndc1 = pt1.xy * invW.y;
    float2 ndc2 = pt2.xy * invW.z;

    // Computing partial derivatives and prospective correct attribute interpolation with barycentric coordinates
	// Equation for calculation taken from Appendix A of DAIS paper:
	// https://cg.ivd.kit.edu/publications/2015/dais/DAIS.pdf
    
    // Calculating inverse of determinant(rcp of area of triangle).
    float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));
    
    // Determine the partial derivatives
    result.ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
    result.ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;

    // Sum of partial derivatives
    float ddxSum = dot(result.ddx, float3(1.f, 1.f, 1.f));
    float ddySum = dot(result.ddy, float3(1.f, 1.f, 1.f));
    
    // Delta vector from pixel's screen position to vertex 0 of the triangle.
    float2 deltaVec = pixelNdc - ndc0;
    
    // Calculate interpolated W at point
    float interpInvW = invW.x + deltaVec.x * ddxSum + deltaVec.y * ddySum;
    float interpW = rcp(interpInvW);
    
    // The barycentric co-ordinate (lambda) is determined by perspective-correct interpolation. 
	// Equation taken from DAIS paper.
    result.lambda.x = interpW * (invW[0] + deltaVec.x * result.ddx.x + deltaVec.y * result.ddy.x);
    result.lambda.y = interpW * (0.f     + deltaVec.x * result.ddx.y + deltaVec.y * result.ddy.y);
    result.lambda.z = interpW * (0.f     + deltaVec.x * result.ddx.z + deltaVec.y * result.ddy.z);

    //Scaling from NDC to pixel units
    result.ddx *= (2.f / winSize.x);
    result.ddy *= (2.f / winSize.y);
    ddxSum     *= (2.f / winSize.x);
    ddySum     *= (2.f / winSize.y);
    
    // This part fixes the derivatives error happening for the projected triangles.
	// Instead of calculating the derivatives constantly across the 2D triangle we use a projected version
	// of the gradients, this is more accurate and closely matches GPU raster behavior.
	// Final gradient equation: ddx = (((lambda/w) + ddx) / (w+|ddx|)) - lambda
    
    // Calculating interpW at partial derivatives position sum.
    float interpWddx = 1.f / (interpInvW + ddxSum);
    float interpWddy = 1.f / (interpInvW + ddySum);

    // Calculating perspective projected derivatives.
    result.ddx = interpWddx * (result.lambda * interpInvW + result.ddx) - result.lambda;
    result.ddy = interpWddy * (result.lambda * interpInvW + result.ddy) - result.lambda;

    return result;
}

BarycentricDeriv CalculateRayBary(float3 pt0, float3 pt1, float3 pt2, float3 position, float3 positionDX, float3 positionDY, float3 camPos)
{
    BarycentricDeriv result;

    float3 currRay = position - camPos;
    float3 rayDX = positionDX - camPos;
    float3 rayDY = positionDY - camPos;
    
    float3 H = RayTriangleIntersection(pt0, pt1, pt2, camPos, normalize(currRay));
    float3 Hx = RayTriangleIntersection(pt0, pt1, pt2, camPos, normalize(rayDX));
    float3 Hy = RayTriangleIntersection(pt0, pt1, pt2, camPos, normalize(rayDY));
    
    result.lambda = H;
    
    result.ddx = Hx - H;
    result.ddy = Hy - H;
    
    return result;
}

float InterpolateWithDerivatives(BarycentricDeriv derivatives, float3 v)
{
    return dot(v, derivatives.lambda);
}

float3 InterpolateWithDerivatives(BarycentricDeriv derivatives, float3x3 attributes)
{
    float3 attr0 = attributes[0];
    float3 attr1 = attributes[1];
    float3 attr2 = attributes[2];

    return float3(dot(attr0, derivatives.lambda), dot(attr1, derivatives.lambda), dot(attr2, derivatives.lambda));
}

GradientInterpolationResults Interpolate2DWithDerivatives(BarycentricDeriv derivatives, float3x2 attributes)
{
    float3 attr0 = GetRowInMatrix(attributes, 0);
    float3 attr1 = GetRowInMatrix(attributes, 1);
    
    GradientInterpolationResults result;
    
    result.interpolated.x = InterpolateWithDerivatives(derivatives, attr0);
    result.interpolated.y = InterpolateWithDerivatives(derivatives, attr1);

    result.dx.x = dot(attr0, derivatives.ddx);
    result.dx.y = dot(attr1, derivatives.ddx);
    result.dy.x = dot(attr0, derivatives.ddy);
    result.dy.y = dot(attr1, derivatives.ddy);
    
    return result;
}






struct PartialDerivatives
{
    float3 lambda;
    float3 ddx;
    float3 ddy;
};

struct UVGradient
{
    float2 uv;
    float2 ddx;
    float2 ddy;
};

PartialDerivatives CalculateDerivatives(in float4 clipPositions[3], in float2 ndcUV, in float2 resolution)
{
    PartialDerivatives result;

    const float3 invW = rcp(float3(clipPositions[0].w, clipPositions[1].w, clipPositions[2].w));
    const float2 ndc0 = clipPositions[0].xy * invW.x;
    const float2 ndc1 = clipPositions[1].xy * invW.y;
    const float2 ndc2 = clipPositions[2].xy * invW.z;

    const float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));
    result.ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
    result.ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;

    float ddxSum = dot(result.ddx, 1.f);
    float ddySum = dot(result.ddy, 1.f);

    const float2 deltaV = ndcUV - ndc0;
    const float interpInvW = invW.x + deltaV.x * ddxSum + deltaV.y * ddySum;
    const float interpW = 1.f / interpInvW;

    result.lambda.x = interpW * (invW[0] + deltaV.x * result.ddx.x + deltaV.y * result.ddy.x);
    result.lambda.y = interpW * (0.0f    + deltaV.x * result.ddx.y + deltaV.y * result.ddy.y);
    result.lambda.z = interpW * (0.0f    + deltaV.x * result.ddx.z + deltaV.y * result.ddy.z);

    const float2 twoOverRes = 2.f / resolution;

    result.ddx *= twoOverRes.x;
    result.ddy *= twoOverRes.y;
    ddxSum *= twoOverRes.x;
    ddySum *= twoOverRes.y;

    const float interpDdxW = 1.f / (interpInvW + ddxSum);
    const float interpDdyW = 1.f / (interpInvW + ddySum);

    result.ddx = interpDdxW * (result.lambda * interpInvW + result.ddx) - result.lambda;
    result.ddy = interpDdyW * (result.lambda * interpInvW + result.ddy) - result.lambda;

    return result;
}

float3 Interpolate(in PartialDerivatives derivatives, in float3 values)
{
    return float3(dot(values, derivatives.lambda), dot(values, derivatives.ddx), dot(values, derivatives.ddy));
}

UVGradient CalculateUVGradient(in PartialDerivatives derivatives, in float2 uvs[3])
{
    const float3 interpUVs[] = 
    {
        Interpolate(derivatives, float3(uvs[0].x, uvs[1].x, uvs[2].x)),
        Interpolate(derivatives, float3(uvs[0].y, uvs[1].y, uvs[2].y)),
    };

    UVGradient result;
    result.uv = float2(interpUVs[0].x, interpUVs[1].x);
    result.ddx = float2(interpUVs[0].y, interpUVs[1].y);
    result.ddy = float2(interpUVs[0].z, interpUVs[1].z);

    return result;
}

float3 InterpolateFloat3(in PartialDerivatives derivatives, in float3 data[3])
{
    return float3
                (
                    Interpolate(derivatives, float3(data[0].x, data[1].x, data[2].x)).x,
                    Interpolate(derivatives, float3(data[0].y, data[1].y, data[2].y)).x,
                    Interpolate(derivatives, float3(data[0].z, data[1].z, data[2].z)).x
                );
}