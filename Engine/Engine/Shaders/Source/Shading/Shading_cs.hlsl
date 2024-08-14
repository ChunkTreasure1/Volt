#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"
#include "Gradients.hlsli"

#include "PBR.hlsli"

namespace ShadingMode
{
    static const uint Shaded = 0;
    static const uint Albedo = 1;
    static const uint Normals = 2;
    static const uint Metalness = 3;
    static const uint Roughness = 4;
    static const uint Emissive = 5;
    static const uint AO = 6;

    static const uint VisualizeCascades = 7;
    static const uint VisualizeLightComplexity = 8;
}

namespace VisualizationMode
{
    static const uint VisualizeCascades = 1;
    static const uint VisualizeLightComplexity = 2;
}

struct Constants
{
    vt::UniformRWTex2D<float4> output;
    
    vt::UniformTex2D<float4> albedo;
    vt::UniformTex2D<float4> normals;
    vt::UniformTex2D<float2> material;
    vt::UniformTex2D<float3> emissive;
    vt::UniformTex2D<uint> aoTexture;
    vt::UniformTex2D<float> depthTexture;

    uint shadingMode;
    uint visualizationMode;

    PBRConstants pbrConstants;
};

float CalculateAO(vt::UniformTex2D<uint> aoTex, uint2 pixelCoord)
{
#define XE_GTAO_OCCLUSION_TERM_SCALE (1.5f)      // for packing in UNORM (because raw, pre-denoised occlusion term can overshoot 1 but will later average out to 1)

    const float ao = (aoTex.Load(int3(pixelCoord, 0)).x >> 24) / 255.f;
    float finalAO = min(ao * XE_GTAO_OCCLUSION_TERM_SCALE, 1.f);

    return finalAO;
}

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.pbrConstants.viewData.Load();

    uint2 size = viewData.renderSize;
    
    if (any(threadId.xy >= size))
    {
        return;
    }
    
    const float4 albedo = constants.albedo.Load(int3(threadId.xy, 0));
    
    if (albedo.a < 0.5f)
    {
        return;
    }
    
    const float2 texCoords = float2(float(threadId.x) * viewData.invRenderSize.x, 1.f - float(threadId.y) * viewData.invRenderSize.y);
    
    const float2 material = constants.material.Load(int3(threadId.xy, 0));
    
    const float metallic = material.x;
    const float roughness = material.y;
    const float3 emissive = constants.emissive.Load(int3(threadId.xy, 0));
    const float3 normal = normalize(constants.normals.Load(int3(threadId.xy, 0)).xyz * 2.f - 1.f);
    const float ao = CalculateAO(constants.aoTexture, threadId.xy);    

    const float pixelDepth = constants.depthTexture.Load(int3(threadId.xy, 0));
    const float3 worldPosition = ReconstructWorldPosition(viewData, texCoords, pixelDepth);
    
    PBRInput pbrInput;
    pbrInput.albedo = albedo;
    pbrInput.normal = normal;
    pbrInput.metallic = metallic;
    pbrInput.roughness = roughness;
    pbrInput.emissive = emissive;
    pbrInput.worldPosition = worldPosition;
    pbrInput.ao = ao;
    pbrInput.tileId = threadId.xy / LIGHT_CULLING_TILE_SIZE;
    
    float3 outputColor = 1.f;
    
    switch (constants.shadingMode)
    {
        case ShadingMode::Shaded:
        {
            outputColor = CalculatePBR(pbrInput, constants.pbrConstants);
            break;
        }

        case ShadingMode::Albedo:
        {
            outputColor = albedo.rgb;
            break;
        }

        case ShadingMode::Normals:
        {
            outputColor = normal * 0.5f + 0.5f;
            break;
        }

        case ShadingMode::Metalness:
        {
            outputColor = metallic;
            break;
        }

        case ShadingMode::Roughness:
        {
            outputColor = roughness;
            break;
        }

        case ShadingMode::Emissive:
        {
            outputColor = emissive;
            break;
        }

        case ShadingMode::AO:
        {
            outputColor = ao;
            break;
        }

        default:
            break;
    };

    switch (constants.visualizationMode)
    {
        case VisualizationMode::VisualizeCascades:
        {
            outputColor = outputColor * 0.2f + GetCascadeColorFromIndex(GetCascadeIndexFromWorldPosition(constants.pbrConstants.directionalLight.Load(), worldPosition, viewData.view)) * 0.5f;
            break;
        }

        case VisualizationMode::VisualizeLightComplexity:
        {
            outputColor = outputColor * 0.2f + GetLightComplexityGradient(GetLightCount(constants.pbrConstants.visiblePointLights, viewData.tileCountX, pbrInput.tileId));
            break;
        }

        default:
            break;
    };

    constants.output.Store(threadId.xy, float4(outputColor, 1.f));
}