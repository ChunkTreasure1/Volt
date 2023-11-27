#include "Defines.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Structures.hlsli"
#include "Utility.hlsli"

struct Constants
{
    TextureT<float4> albedo;
    TextureT<float4> materialEmissive;
    TextureT<float4> normalEmissive;
    
    RWTexture<float4> output;
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
    
    const float4 materialEmissive = constants.materialEmissive.Load2D(int3(threadId.xy, 0));
    const float4 normalEmissive = constants.normalEmissive.Load2D(int3(threadId.xy, 0));
    
    const float3 sunDir = normalize(float3(0.f, 1.f, 0.f));
    const float attenuation = max(dot(normalEmissive.xyz, sunDir), 0.f);
    
    const float3 ambiance = 0.1f;
    const float3 outputColor = albedo.xyz * attenuation + ambiance;
    
    constants.output.Store2D(threadId.xy, float4(outputColor.xyz, 1.f));
}