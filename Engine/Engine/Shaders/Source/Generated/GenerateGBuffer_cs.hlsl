#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"
#include "VectorUtility.hlsli"

#include "MeshletHelpers.hlsli"

struct Constants
{
    TTexture<uint> visibilityBuffer;
    TypedBuffer<uint> materialCountBuffer;
    TypedBuffer<uint> materialStartBuffer;
    TypedBuffer<uint2> pixelCollection;
    
    TypedBuffer<GPUScene> gpuScene;
    UniformBuffer<ViewData> viewData;
    
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

struct MaterialEvaluationData
{
    float2 texCoords;
    float2 texCoordsDX;
    float2 texCoordsDY;
};

struct EvaluatedMaterial
{
    float4 albedo;
    float roughness;
    float metallic;
    float3 normal;
    float3 emissive;
    
    void Setup()
    {
        albedo = 1.f;
        roughness = 0.9f;
        metallic = 0.f;
        normal = float3(0.5f, 0.5f, 1.f);
        emissive = 0.f;
    }
};

EvaluatedMaterial EvaluateMaterial(in GPUMaterial material, in MaterialEvaluationData evalData)
{
    GENERATED_SHADER
}

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex)
{
    const Constants constants = GetConstants<Constants>();
    const GPUScene scene = constants.gpuScene.Load(0);
    const ViewData viewData = constants.viewData.Load();
    
    if (groupThreadIndex == 0)
    {
        m_materialCount = constants.materialCountBuffer.Load(constants.materialId);
        m_materialStart = constants.materialStartBuffer.Load(constants.materialId);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    const uint pixelIndex = m_materialStart + threadId.x;
    
    if (threadId.x >= m_materialCount)
    {
        return;
    }
    
    const float2 pixelPosition = constants.pixelCollection.Load(pixelIndex) + 0.5f;
    const uint visibilityValues = constants.visibilityBuffer.Load2D(int3(pixelPosition, 0));
    
    const uint triangleId = UnpackTriangleID(visibilityValues);
    const uint meshletId = UnpackMeshletID(visibilityValues);
    
    const Meshlet meshlet = scene.meshletsBuffer.Load(meshletId);
    const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(meshlet.objectId);
    const GPUMesh mesh = scene.meshesBuffer.Load(meshlet.meshId);
    
    const uint triIndex0 = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + triangleId + 0) + meshlet.vertexOffset + mesh.vertexStartOffset;
    const uint triIndex1 = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + triangleId + 1) + meshlet.vertexOffset + mesh.vertexStartOffset;
    const uint triIndex2 = mesh.meshletIndexBuffer.Load(mesh.meshletIndexStartOffset + meshlet.triangleOffset + triangleId + 2) + meshlet.vertexOffset + mesh.vertexStartOffset;
    
    const VertexPositionData vPos0 = mesh.vertexPositionsBuffer.Load(triIndex0);
    const VertexPositionData vPos1 = mesh.vertexPositionsBuffer.Load(triIndex1);
    const VertexPositionData vPos2 = mesh.vertexPositionsBuffer.Load(triIndex2);
    
    const float4 wPos0 = mul(drawData.transform, float4(vPos0.position, 1.f));
    const float4 wPos1 = mul(drawData.transform, float4(vPos1.position, 1.f));
    const float4 wPos2 = mul(drawData.transform, float4(vPos2.position, 1.f));
    
    const float2 screenPos = float2((pixelPosition.x / constants.viewSize.x) * 2.f - 1.f, -(pixelPosition.y / constants.viewSize.y) * 2.f + 1.f);
    
    float4 pos0 = mul(viewData.viewProjection, wPos0);
    float4 pos1 = mul(viewData.viewProjection, wPos1);
    float4 pos2 = mul(viewData.viewProjection, wPos2);
    
    const float3 oneOverW = 1.f / float3(pos0.w, pos1.w, pos2.w);
    
    pos0 *= oneOverW[0];
    pos1 *= oneOverW[1];
    pos2 *= oneOverW[2];
    
    BarycentricDeriv derivatives = CalculateFullBary(pos0, pos1, pos2, screenPos, constants.viewSize);
    
    // Interpolated W for all three vertices
    float interpW = 1.f / dot(oneOverW, derivatives.lambda);
    
    // Reconstruct pixel Z value
    float z = interpW * viewData.projection[2][2] + viewData.projection[3][2];
    
    // Calculate interpolated world position
    float3 worldPosition = mul(viewData.inverseViewProjection, float4(screenPos * interpW, z, interpW)).xyz;
    
    float3x2 triTexCoords = 0.f;
    float3x3 triNormals = 0.f;
    float3x3 triTangents = 0.f;
    
#if !USE_RAY_DIFFERENTIALS
    const float3 perspectiveCorrection = oneOverW;
#else
    const float3 perspectiveCorrection = 1.f;    
#endif
    {
        const VertexMaterialData material0 = mesh.vertexMaterialBuffer.Load(triIndex0);
        const VertexMaterialData material1 = mesh.vertexMaterialBuffer.Load(triIndex1);
        const VertexMaterialData material2 = mesh.vertexMaterialBuffer.Load(triIndex2);
        
        triTexCoords = float3x2(material0.texCoords * perspectiveCorrection[0], material1.texCoords * perspectiveCorrection[1], material2.texCoords * perspectiveCorrection[2]);
     
#if USE_RAY_DIFFERENTIALS
        const float2 twoOverRes = 2.f / constants.viewSize;
        
        float3 positionDX = mul(viewData.inverseViewProjection, float4((screenPos + twoOverRes.x / 2.f) * interpW, z, interpW)).xyz;
        float3 positionDY = mul(viewData.inverseViewProjection, float4((screenPos + twoOverRes.y / 2.f) * interpW, z, interpW)).xyz;
        
        derivatives = CalculateRayBary(wPos0.xyz, wPos1.xyz, wPos2.xyz, worldPosition, positionDX, positionDY, viewData.cameraPosition.xyz);
#endif
        
        const float3 triNormal0 = DecodeNormal(material0.normal);
        const float3 triNormal1 = DecodeNormal(material1.normal);
        const float3 triNormal2 = DecodeNormal(material2.normal);
        
        triNormals = MakeF3X3FromRows(triNormal0 * perspectiveCorrection[0], triNormal1 * perspectiveCorrection[1], triNormal2 * perspectiveCorrection[2]);
        triTangents = MakeF3X3FromRows(DecodeTangent(triNormal0, material0.tangent) * perspectiveCorrection[0], DecodeTangent(triNormal1, material1.tangent) * perspectiveCorrection[1], DecodeTangent(triNormal2, material2.tangent) * perspectiveCorrection[2]);
    }
    
    GradientInterpolationResults results = Interpolate2DWithDerivatives(derivatives, triTexCoords);
    
    float linearZ = LinearizeDepth01(z / interpW, viewData);
    float mip = pow(pow(linearZ, 0.9f) * 5.0f, 1.5f);
    
    float2 texCoordsDX = results.dx * mip;
    float2 texCoordsDY = results.dy * mip;
    float2 texCoords = results.interpolated;
    
#if !USE_RAY_DIFFERENTIALS
    texCoords *= interpW;
    texCoordsDX *= interpW;
    texCoordsDY *= interpW;
#endif    

    const float3x3 worldRotationMatrix = (float3x3)drawData.transform;
    
    const float3 tangent = normalize(mul(worldRotationMatrix, normalize(InterpolateWithDerivatives(derivatives, triTangents))));
    const float3 normal = normalize(mul(worldRotationMatrix, normalize(InterpolateWithDerivatives(derivatives, triNormals))));
    
    const float3x3 TBN = CalculateTBN(normal, tangent);
    
    const GPUMaterial material = scene.materialsBuffer.Load(constants.materialId);
    
    MaterialEvaluationData evalData;
    evalData.texCoords = texCoords;
    evalData.texCoordsDX = texCoordsDX;
    evalData.texCoordsDY = texCoordsDY;
    
    EvaluatedMaterial evaluatedMaterial = EvaluateMaterial(material, evalData);
    
    float3 resultNormal = evaluatedMaterial.normal.xyz * 2.f - 1.f;
    resultNormal.z = sqrt(1.f - saturate(resultNormal.x * resultNormal.x + resultNormal.y * resultNormal.y));
    resultNormal = normalize(mul(TBN, normalize(resultNormal)));
    
    float4 albedo = evaluatedMaterial.albedo;
    albedo.xyz = SRGBToLinear(albedo.xyz);
    
    const float4 materialEmissive = float4(evaluatedMaterial.metallic, evaluatedMaterial.roughness, evaluatedMaterial.emissive.x, evaluatedMaterial.emissive.y);
    const float4 normalEmissive = float4(resultNormal, evaluatedMaterial.emissive.z);
    
    constants.albedo.Store2D(pixelPosition, albedo);
    constants.materialEmissive.Store2D(pixelPosition, materialEmissive);
    constants.normalEmissive.Store2D(pixelPosition, normalEmissive);
}