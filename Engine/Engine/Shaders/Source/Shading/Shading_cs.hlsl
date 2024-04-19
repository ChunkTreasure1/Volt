#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"

#include "PBR.hlsli"

namespace ShadingMode
{
    static const uint Shaded = 0;
    static const uint Albedo = 1;
    static const uint Normals = 2;
    static const uint Metalness = 3;
    static const uint Roughness = 4;
    static const uint Emissive = 5;

    static const uint VisualizeCascades = 6;
}

struct Constants
{
    UniformRWTexture<float4> output;
    
    UniformTexture<float4> albedo;
    UniformTexture<float4> normals;
    UniformTexture<float2> material;
    UniformTexture<float3> emissive;
    UniformTexture<float> depthTexture;

    uint shadingMode;

    PBRConstants pbrConstants;
};

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint groupThreadIndex : SV_GroupIndex)
{
    const Constants constants = GetConstants<Constants>();

    uint2 size;
    constants.output.GetDimensions(size.x, size.y);
    
    if (any(threadId.xy >= size))
    {
        return;
    }
    
    const float4 albedo = constants.albedo.Load2D(int3(threadId.xy, 0));
    
    if (albedo.a < 0.5f)
    {
        return;
    }
    
    const ViewData viewData = constants.pbrConstants.viewData.Load();
    const float2 texCoords = float2(float(threadId.x) * viewData.invRenderSize.x, 1.f - float(threadId.y) * viewData.invRenderSize.y);
    
    const float2 material = constants.material.Load2D(int3(threadId.xy, 0));
    
    const float metallic = material.x;
    const float roughness = material.y;
    const float3 emissive = constants.emissive.Load2D(int3(threadId.xy, 0));
    const float3 normal = normalize(constants.normals.Load2D(int3(threadId.xy, 0)).xyz * 2.f - 1.f);
    
    const float pixelDepth = constants.depthTexture.Load2D(int3(threadId.xy, 0));
    const float3 worldPosition = ReconstructWorldPosition(viewData, texCoords, pixelDepth);
    
    PBRInput pbrInput;
    pbrInput.albedo = albedo;
    pbrInput.normal = normal;
    pbrInput.metallic = metallic;
    pbrInput.roughness = roughness;
    pbrInput.emissive = emissive;
    pbrInput.worldPosition = worldPosition;
    
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

        case ShadingMode::VisualizeCascades:
        {
            outputColor = CalculatePBR(pbrInput, constants.pbrConstants);
            outputColor = outputColor * 0.2f + GetCascadeColorFromIndex(GetCascadeIndexFromWorldPosition(constants.pbrConstants.directionalLight.Load(0), worldPosition, viewData.view)) * 0.5f;
            break;
        }

        default:
            break;
    };

    constants.output.Store2D(threadId.xy, float4(LinearToSRGB(outputColor), 1.f));
}