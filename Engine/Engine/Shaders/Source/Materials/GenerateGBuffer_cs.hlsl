#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"

struct Constants
{
    TextureT<uint2> visibilityBuffer;
    TypedBuffer<uint> materialCountBuffer;
    TypedBuffer<uint> materialStartBuffer;
    TypedBuffer<uint2> pixelCollection;
    
    TypedBuffer<GPUScene> gpuScene;
    TypedBuffer<CameraData> cameraData;
    
    RWTexture<float4> albedo;
    RWTexture<float4> materialEmissive;
    RWTexture<float4> normalEmissive;
    
    uint materialId;
    
    float2 viewSize; // Move to buffer
};

groupshared uint m_materialCount;
groupshared uint m_materialStart;

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

bool RayPlaneIntersection(out float hitT, float3 rayOrigin, float3 rayDir, float3 planeSurfacePoint, float3 planeNormal)
{
    const float denom = dot(rayDir, planeNormal);
    const float numerator = dot(planeSurfacePoint - rayOrigin, planeNormal);
    
    if (denom == 0.f)
    {
        if (numerator == 0.f)
        {
            hitT = 0.f;
            return true;
        }
        else
        {
            hitT = -1.f;
            return false;
        }
    }
    
    hitT = numerator / denom;
    return true;
}

float3 GetBarycentricsFromPlanePoint(float3 pt, float3 v0, float3 v1, float3 v2)
{
    const float3 e0 = v1 - v0;
    const float3 e1 = v2 - v0;
    const float3 e2 = pt - v0;
    
    const float d00 = dot(e0, e0);
    const float d01 = dot(e0, e1);
    const float d11 = dot(e1, e1);
    const float d20 = dot(e2, e0);
    const float d21 = dot(e2, e1);

    const float denom = 1.f / (d00 * d11 - d01 * d01);
    const float v = (d11 * d20 - d01 * d21) * denom;
    const float w = (d00 * d21 - d01 * d20) * denom;
    const float u = 1.f - v - w;
    
    return float3(u, v, w);
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

BarycentricDeriv CalculateFullBary(float3 pt0, float3 pt1, float3 pt2, float3 position, float3 positionDX, float3 positionDY, float3 camPos)
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
    float3 attr0 = float3(attributes[0].x, attributes[1].x, attributes[2].x);
    float3 attr1 = float3(attributes[0].y, attributes[1].y, attributes[2].y);
    
    GradientInterpolationResults result;
    
    result.interpolated.x = InterpolateWithDerivatives(derivatives, attr0);
    result.interpolated.y = InterpolateWithDerivatives(derivatives, attr1);

    result.dx.x = dot(attr0, derivatives.ddx);
    result.dx.y = dot(attr1, derivatives.ddx);
    result.dy.x = dot(attr0, derivatives.ddy);
    result.dy.y = dot(attr1, derivatives.ddy);
    
    return result;
}

const float3 DecodeNormal(uint normalInt)
{
    uint2 octIntNormal;
    
    octIntNormal.x = (normalInt >> 8) & 0xFF;
    octIntNormal.y = (normalInt >> 0) & 0xFF;
    
    float2 octNormal = 0.f;
    octNormal.x = octIntNormal.x / 255.f;
    octNormal.y = octIntNormal.y / 255.f;
    
    return OctNormalDecode(octNormal);
}

const float3 DecodeTangent(float3 normal, float tangentFloat)
{
    return decode_tangent(normal, tangentFloat);
}

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex)
{
    const Constants constants = GetConstants<Constants>();
    const GPUScene scene = constants.gpuScene.Load(0);
    const CameraData cameraData = constants.cameraData.Load(0);
    
    if (groupThreadIndex == 0)
    {
        m_materialCount = constants.materialCountBuffer.Load(constants.materialId);
        m_materialStart = constants.materialStartBuffer.Load(constants.materialId);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    const uint pixelIndex = m_materialStart + threadId.x;
    
    if (pixelIndex >= m_materialCount)
    {
        return;
    }
    
    const float2 pixelPosition = constants.pixelCollection.Load(pixelIndex) + 0.5f;
    
    constants.albedo.Store2D(pixelPosition, 1.f);
    return;
    
    
    const uint2 visibilityValues = constants.visibilityBuffer.Load2D(int3(pixelPosition, 0));
    
    const uint objectId = visibilityValues.x;
    const uint triangleId = visibilityValues.y;
    
    const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    
    const uint triIndex0 = mesh.indexBuffer.Load(triangleId * 3 + 0) + mesh.vertexStartOffset;
    const uint triIndex1 = mesh.indexBuffer.Load(triangleId * 3 + 1) + mesh.vertexStartOffset;
    const uint triIndex2 = mesh.indexBuffer.Load(triangleId * 3 + 2) + mesh.vertexStartOffset;
    
    const VertexPositionData vPos0 = mesh.vertexPositionsBuffer.Load(triIndex0);
    const VertexPositionData vPos1 = mesh.vertexPositionsBuffer.Load(triIndex1);
    const VertexPositionData vPos2 = mesh.vertexPositionsBuffer.Load(triIndex2);
    
    const float4 wPos0 = mul(drawData.transform, float4(vPos0.position, 1.f));
    const float4 wPos1 = mul(drawData.transform, float4(vPos1.position, 1.f));
    const float4 wPos2 = mul(drawData.transform, float4(vPos2.position, 1.f));
    
    // NDC positions
    const float nearPlaneDist = 0.1f;
    const float4 projectedScreenPosition = float4((pixelPosition.x / constants.viewSize.x) * 2.f - 1.f, -(pixelPosition.y / constants.viewSize.y) * 2.f + 1.f, 0.f, 1.f) * nearPlaneDist;
    const float4 unprojectedScreenPosition = mul(cameraData.inverseViewProjection, projectedScreenPosition);
    
    const float3 rayDir = normalize(unprojectedScreenPosition.xyz);
    
    const float3 neighbourRayDirX = QuadReadAcrossX(rayDir);
    const float3 neighbourRayDirY = QuadReadAcrossY(rayDir);
    
    float4 edge1 = wPos1 - wPos0;
    float4 edge2 = wPos2 - wPos0;

    const float3 triNormal = cross(edge1.xyz, edge2.xyz);
    
    float hitT;
    RayPlaneIntersection(hitT, cameraData.position.xyz, rayDir, wPos0.xyz, triNormal);
    
    const float3 hitPoint = cameraData.position.xyz + rayDir * hitT;
    const float3 barycentrics = GetBarycentricsFromPlanePoint(hitPoint, wPos0.xyz, wPos1.xyz, wPos2.xyz);
    
    const VertexMaterialData materialData0 = mesh.vertexMaterialBuffer.Load(triIndex0);
    const VertexMaterialData materialData1 = mesh.vertexMaterialBuffer.Load(triIndex1);
    const VertexMaterialData materialData2 = mesh.vertexMaterialBuffer.Load(triIndex2);
    
    const float2 texCoords = barycentrics.x * materialData0.texCoords + barycentrics.y * materialData1.texCoords + barycentrics.z * materialData2.texCoords;
    
    float2 dUVdx;
    float2 dUVdy;
    
    const bool2 allSameTri = WaveActiveAllEqual(visibilityValues);
    const bool allActive = WaveActiveCountBits(true) == WaveGetLaneCount();
    
    if (allSameTri.x && allSameTri.y && allActive)
    {
        dUVdx = ddx(texCoords);
        dUVdy = ddy(texCoords);
    }
    else
    {
        // Compute hitpoints for neighboring camera rays against the same triangle
        float hitTX, hitTY;
        RayPlaneIntersection(hitTX, cameraData.position.xyz, neighbourRayDirX, wPos0.xyz, triNormal);
        RayPlaneIntersection(hitTY, cameraData.position.xyz, neighbourRayDirY, wPos0.xyz, triNormal);
        
        const float3 hitPointX = cameraData.position.xyz + neighbourRayDirX * hitTX;
        const float3 hitPointY = cameraData.position.xyz + neighbourRayDirY * hitTY;
        
        const float3 barycentricsX = GetBarycentricsFromPlanePoint(hitPointX, wPos0.xyz, wPos1.xyz, wPos2.xyz);
        const float3 barycentricsY = GetBarycentricsFromPlanePoint(hitPointY, wPos0.xyz, wPos1.xyz, wPos2.xyz);
        
        const float2 texCoordsX = barycentricsX.x * materialData0.texCoords + barycentricsX.y * materialData1.texCoords + barycentricsX.z * materialData2.texCoords;
        const float2 texCoordsY = barycentricsY.x * materialData0.texCoords + barycentricsY.y * materialData1.texCoords + barycentricsY.z * materialData2.texCoords;
        
        dUVdx = texCoordsX - texCoords;
        dUVdy = texCoordsY - texCoords;
    }
    
#if 0
    const float2 screenPos = float2((pixelPosition.x / constants.viewSize.x) * 2.f - 1.f, -(pixelPosition.y / constants.viewSize.y) * 2.f + 1.f);
    
    float4 pos0 = mul(cameraData.viewProjection, wPos0);
    float4 pos1 = mul(cameraData.viewProjection, wPos1);
    float4 pos2 = mul(cameraData.viewProjection, wPos2);
    
    const float3 oneOverW = 1.f / float3(pos0.w, pos1.w, pos2.w);
    
    pos0 *= oneOverW[0];
    pos1 *= oneOverW[1];
    pos2 *= oneOverW[2];
    
    BarycentricDeriv derivatives = CalculateFullBary(pos0, pos1, pos2, screenPos, constants.viewSize);
    
    // Interpolated W for all three vertices
    float interpW = 1.f / dot(oneOverW, derivatives.lambda);
    
    // Reconstruct pixel Z value
    float z = interpW * cameraData.projection[2][2] + cameraData.projection[3][2];
    
    // Calculate interpolated world position
    float3 worldPosition = mul(cameraData.inverseViewProjection, float4(screenPos * interpW, z, interpW)).xyz;
    
#if 1
    float3 positionDX = mul(cameraData.inverseViewProjection, float4((screenPos + constants.viewSize.x / 2.f) * interpW, z, interpW)).xyz;
    float3 positionDY = mul(cameraData.inverseViewProjection, float4((screenPos + constants.viewSize.y / 2.f) * interpW, z, interpW)).xyz;
    
    derivatives = CalculateFullBary(wPos0.xyz, wPos1.xyz, wPos2.xyz, worldPosition, positionDX, positionDY, cameraData.position.xyz);
#else
    derivatives = CalculateFullBary(pos0, pos1, pos2, screenPos, constants.viewSize);
#endif
    
    float3x2 triTexCoords = 0.f;
    float3x3 triTangents = 0.f;
    float3x3 triNormals = 0.f;
    
    {
        const VertexMaterialData materialData0 = mesh.vertexMaterialBuffer.Load(triIndex0);
        const VertexMaterialData materialData1 = mesh.vertexMaterialBuffer.Load(triIndex1);
        const VertexMaterialData materialData2 = mesh.vertexMaterialBuffer.Load(triIndex2);
    
        triTexCoords[0] = materialData0.texCoords /** oneOverW[0]*/;
        triTexCoords[1] = materialData1.texCoords /** oneOverW[1]*/;
        triTexCoords[2] = materialData2.texCoords /** oneOverW[2]*/;
        
        triNormals[0] = DecodeNormal(materialData0.normal);
        triNormals[1] = DecodeNormal(materialData1.normal);
        triNormals[2] = DecodeNormal(materialData2.normal);
        
        triTangents[0] = DecodeTangent(triNormals[0], materialData0.tangent) * oneOverW[0];
        triTangents[1] = DecodeTangent(triNormals[1], materialData1.tangent) * oneOverW[1];
        triTangents[2] = DecodeTangent(triNormals[2], materialData2.tangent) * oneOverW[2];
        
        triNormals[0] *= oneOverW[0];
        triNormals[0] *= oneOverW[1];
        triNormals[0] *= oneOverW[2];
    }
    
    GradientInterpolationResults texCoordResults = Interpolate2DWithDerivatives(derivatives, triTexCoords);
    
    const float linearZ = LinearizeDepth(z / interpW, cameraData);
    const float mip = pow(pow(linearZ, 0.9f) * 5.f, 1.5f);
    
    float2 texCoordDX = texCoordResults.dx * mip;
    float2 texCoordDY = texCoordResults.dy * mip;
    float2 texCoords = texCoordResults.interpolated;
    
    texCoords *= interpW;
    texCoordDX *= interpW;
    texCoordDY *= interpW;
    
    const float3 tangent = normalize(InterpolateWithDerivatives(derivatives, triTangents));
    const float3 normal = normalize(InterpolateWithDerivatives(derivatives, triNormals));
    const float3 binormal = normalize(cross(tangent, normal));
#endif
    
    const float4 albedo = 1.f;
    const float4 materialEmissive = float4(0.f, 0.9f, 0.f, 0.f);
    const float4 normalEmissive = float4(0.f, 0.f, 0.f, 1.f);
    
    constants.albedo.Store2D(pixelPosition, float4(texCoords, 0.f, 1.f));
    constants.materialEmissive.Store2D(pixelPosition, materialEmissive);
    constants.normalEmissive.Store2D(pixelPosition, normalEmissive);
}