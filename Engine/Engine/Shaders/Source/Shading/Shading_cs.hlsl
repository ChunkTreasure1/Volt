#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"

#include "PBR.hlsli"

struct Constants
{
    RWTexture<float4> output;
    
    TTexture<float4> albedo;
    TTexture<float4> materialEmissive;
    TTexture<float4> normalEmissive;
    TTexture<float> depthTexture;

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
    
    const float4 materialEmissive = constants.materialEmissive.Load2D(int3(threadId.xy, 0));
    const float4 normalEmissive = constants.normalEmissive.Load2D(int3(threadId.xy, 0));
    
    const float metallic = materialEmissive.x;
    const float roughness = materialEmissive.y;
    const float3 emissive = float3(materialEmissive.zw, normalEmissive.w);
    const float3 normal = normalEmissive.xyz;
    
    const float pixelDepth = constants.depthTexture.Load2D(int3(threadId.xy, 0));
    const float3 worldPosition = ReconstructWorldPosition(viewData, texCoords, pixelDepth);
    
    PBRInput pbrInput;
    pbrInput.albedo = albedo;
    pbrInput.normal = normal;
    pbrInput.metallic = metallic;
    pbrInput.roughness = roughness;
    pbrInput.emissive = emissive;
    pbrInput.worldPosition = worldPosition;
    
    const float3 outputColor = CalculatePBR(pbrInput, constants.pbrConstants);
    
    constants.output.Store2D(threadId.xy, float4(LinearToSRGB(outputColor), 1.f));
}