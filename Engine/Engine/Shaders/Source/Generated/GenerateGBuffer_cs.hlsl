#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"
#include "VectorUtility.hlsli"

#include "MeshletHelpers.hlsli"
#include "Barycentrics.hlsli"
#include "VisibilityBuffer.hlsli"

struct Constants
{
    UniformTexture<uint2> visibilityBuffer;
    UniformTypedBuffer<uint> materialCountBuffer;
    UniformTypedBuffer<uint> materialStartBuffer;
    UniformTypedBuffer<uint2> pixelCollection;
    
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;
    
    UniformRWTexture<float4> albedo;
    UniformRWTexture<float4> normals;
    UniformRWTexture<float2> material;
    UniformRWTexture<float3> emissive;
    
    uint materialId;
    
    float2 viewSize; // Move to buffer
};

groupshared uint m_materialCount;
groupshared uint m_materialStart;

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
    const GPUScene scene = constants.gpuScene;
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
    const uint2 visibilityValues = constants.visibilityBuffer.Load2D(int3(pixelPosition, 0));
    
    const uint objectId = visibilityValues.x;
    const uint triangleId = UnpackTriangleID(visibilityValues.y);
    const uint meshletId = UnpackMeshletID(visibilityValues.y);
    
    const ObjectDrawData drawData = scene.objectDrawDataBuffer.Load(objectId);
    const GPUMesh mesh = scene.meshesBuffer.Load(drawData.meshId);
    const Meshlet meshlet = mesh.meshletsBuffer.Load(meshletId);

    const uint3 meshletTriIndices = UnpackPrimitive(mesh.meshletDataBuffer.Load(meshlet.dataOffset + meshlet.vertexCount + triangleId)); 
    const uint3 triIndices = LoadTriangleIndices(mesh.meshletDataBuffer, meshlet.dataOffset, mesh.vertexStartOffset, meshletTriIndices);
    const PositionData vertexPositions = LoadVertexPositions(mesh.vertexPositionsBuffer, triIndices);

    const float4 worldPositions[] = 
    {
        mul(drawData.transform, float4(vertexPositions.positions[0], 1.f)),
        mul(drawData.transform, float4(vertexPositions.positions[1], 1.f)),
        mul(drawData.transform, float4(vertexPositions.positions[2], 1.f))
    };

    const float4 clipPositions[] =
    {
        mul(viewData.projection, mul(viewData.view, worldPositions[0])),
        mul(viewData.projection, mul(viewData.view, worldPositions[1])),
        mul(viewData.projection, mul(viewData.view, worldPositions[2]))
    };

    const float2 screenPos = float2((pixelPosition.x / constants.viewSize.x) * 2.f - 1.f, -(pixelPosition.y / constants.viewSize.y) * 2.f + 1.f);

    const PartialDerivatives derivatives = CalculateDerivatives(clipPositions, screenPos, constants.viewSize);
    const MaterialData materialData = LoadVertexMaterialData(mesh.vertexMaterialBuffer, triIndices);    
    const UVGradient uvGradient = CalculateUVGradient(derivatives, materialData.texCoords);
    
    const float3x3 worldRotationMatrix = (float3x3)drawData.transform;

    const float3 normal = normalize(mul(worldRotationMatrix, normalize(InterpolateFloat3(derivatives, materialData.normals))));
    const float3 tangent = normalize(mul(worldRotationMatrix, normalize(InterpolateFloat3(derivatives, materialData.tangents))));
    const float3x3 TBN = CalculateTBN(normal, tangent);
    
    const GPUMaterial material = scene.materialsBuffer.Load(constants.materialId);
    
    MaterialEvaluationData evalData;
    evalData.texCoords = uvGradient.uv;
    evalData.texCoordsDX = uvGradient.ddx;
    evalData.texCoordsDY = uvGradient.ddy;
    
    EvaluatedMaterial evaluatedMaterial = EvaluateMaterial(material, evalData);
    
    float3 resultNormal = evaluatedMaterial.normal.xyz * 2.f - 1.f;
    resultNormal.z = sqrt(1.f - saturate(resultNormal.x * resultNormal.x + resultNormal.y * resultNormal.y));
    resultNormal = normalize(mul(TBN, normalize(resultNormal)));
    
    float4 albedo = evaluatedMaterial.albedo;

    // #TODO_Ivar: This depends on the texture format
    albedo.xyz = SRGBToLinear(albedo.xyz);
    
    constants.albedo.Store2D(pixelPosition, albedo);
    constants.normals.Store2D(pixelPosition, float4(resultNormal * 0.5f + 0.5f, 0.f));
    constants.material.Store2D(pixelPosition, float2(evaluatedMaterial.metallic, evaluatedMaterial.roughness));
    constants.emissive.Store2D(pixelPosition, evaluatedMaterial.emissive);
}