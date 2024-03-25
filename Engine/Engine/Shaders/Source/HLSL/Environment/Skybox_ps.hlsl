#include "Common.hlsli"
#include "SamplerStates.hlsli"
#include "FogCommon.hlsli"
#include "Buffers.hlsli"

struct Input
{
    float4 position : SV_Position;
    STAGE_VARIABLE(float3, samplePosition, SAMPLEPOSITION, 0);
    STAGE_VARIABLE(float3, worldPosition, WORLDPOSITION, 1);
};

struct PushConstants
{
    float lod;
    float intensity;
};

[[vk::push_constant]] PushConstants u_pushConstants;

TextureCube u_environmentMap : register(t0, SPACE_MATERIAL);
Texture3D u_volumetricFogTexture : register(BINDING_VOLUMETRIC_FOG_TEXTURE, SPACE_PBR_RESOURCES);

float3 CalculateInscatteredLight(in float3 accumulatedLight, float2 position)
{
    const float4 scatteredLight = u_volumetricFogTexture.Load(int4(position.x / VOXEL_GRID_SIZE_X, position.y / VOXEL_GRID_SIZE_Y, VOXEL_GRID_SIZE_Z - 1, 0));
    const float transmittance = scatteredLight.a;
    
    return accumulatedLight * transmittance + scatteredLight.rgb;
}

float4 main(Input input) : SV_Target0
{
    float4 result = 0.f;
    result = u_environmentMap.SampleLevel(u_linearSampler, input.samplePosition / 100.f, u_pushConstants.lod) * u_pushConstants.intensity;
    result.a = 1.f;
    
    result.rgb += CalculateInscatteredLight(result.rgb, input.position.xy);
    
    return result;
}