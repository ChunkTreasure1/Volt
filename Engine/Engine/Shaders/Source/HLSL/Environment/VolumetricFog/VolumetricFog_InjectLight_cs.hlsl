#include "Defines.hlsli"
#include "Buffers.hlsli"
#include "FogCommon.hlsli"

#include "ShadowMapping.hlsli"

static const float PI = 3.14159265359f;
static const float EPSILON = 0.0001f;

RWTexture3D<float4> o_resultTexture : register(u0, SPACE_OTHER);

Texture2DArray u_directionalShadowMap : register(BINDING_DIR_SHADOWMAP, SPACE_PBR_RESOURCES);
Texture2D u_spotlightShadowMaps[8] : register(BINDING_SPOT_SHADOWMAPS, SPACE_PBR_RESOURCES);

struct PushConstants
{
    float anisotropy;
    float density;
    float globalDensity;
    float padding;
    float4 sizeMultiplier;
    float4 globalColor;
};

[[vk::push_constant]] PushConstants u_pushConstants;

float PhaseFunction(float cosTheta, float g)
{
    float denom = 1.f + g * g + 2.f * g * cosTheta;
    return (1.f / (4.f * PI)) * (1.f - g * g) / max(pow(denom, 1.5f), EPSILON);
}

float Visibility(float3 position)
{
    const uint cascadeIndex = GetCascadeIndexFromWorldPosition(u_directionalLight, position, u_cameraData.view);
    const float3 shadowCoords = GetShadowMapCoords(u_directionalLight.viewProjections[cascadeIndex], position);
    const float2 sampleCoords = float2(shadowCoords.x * 0.5f + 0.5f, -shadowCoords.y * 0.5f + 0.5f);
    
    if (sampleCoords.x > 1.f || sampleCoords.x < 0.f || sampleCoords.y > 1.f || sampleCoords.y < 0.f)
    {
        return 1.f;
    }
    
    const float minShadowBias = 0.0005f / float(cascadeIndex + 1);
    const float shadowMapDepth = u_directionalShadowMap.SampleCmpLevelZero(u_shadowSampler, float3(sampleCoords.xy, (float) cascadeIndex), shadowCoords.z - minShadowBias).r;
    return shadowMapDepth;
}

float3 CalculateDirectionalLightContribution(in float3 worldPos, in float3 dirToCamera)
{
    const float visibilityValue = Visibility(worldPos);
    float3 result = 0.f;
    
    if (visibilityValue > EPSILON)
    {
        result += visibilityValue * u_directionalLight.colorIntensity.xyz * u_directionalLight.colorIntensity.w * PhaseFunction(dot(dirToCamera, u_directionalLight.direction.xyz), u_pushConstants.anisotropy);
    }
    
    return result;
}

float3 CalculatePointLight(in PointLight light, in float3 worldPos, in float3 dirToCamera)
{
    const float3 Li = normalize(light.position.xyz - worldPos);
    const float lightDistance = length(light.position.xyz - worldPos);
        
    float attenuation = clamp(1.f - (lightDistance * lightDistance) / (light.radius * light.radius), 0.f, 1.f);
    attenuation *= lerp(attenuation, 1.f, light.falloff);
        
    const float cosTheta = dot(dirToCamera, Li);
    attenuation *= PhaseFunction(cosTheta, u_pushConstants.anisotropy);
        
    return light.color.xyz * light.intensity * attenuation;
}

int GetPointLightBufferIndex(int i, uint2 tileId)
{
    uint index = tileId.y * u_rendererData.screenTilesCountX + tileId.x;

    uint offset = index * 1024;
    return u_visiblePointLightIndices[offset + i];
}

float3 CalculatePointLightContribution(in float3 worldPos, in float3 dirToCamera, in uint2 tileId)
{
    float3 result = 0.f;
 
    for (uint i = 0; i < 1024; i++)
    {
        uint lightIndex = GetPointLightBufferIndex(i, tileId);
        if (lightIndex == -1)
        {
            break;
        }
    
        result += CalculatePointLight(u_pointLights[lightIndex], worldPos, dirToCamera);
    }
    
    return result;
}

float3 CalculateSpotLight(in SpotLight light, in float3 worldPos, in float3 dirToCamera)
{
    const float lightDistance = length(light.position.xyz - worldPos);
    const float3 Li = normalize(light.position.xyz - worldPos);
        
    const float cutoff = cos(light.angle * 0.5f);
    const float scos = max(dot(Li, light.direction), cutoff);
    const float rim = (1.f - scos) / (1.f - cutoff);
    
    float attenuation = clamp(1.f - (lightDistance * lightDistance) / (light.range * light.range), 0.f, 1.f);
    attenuation *= lerp(attenuation, 1.f, light.falloff);
    attenuation *= 1.f - pow(max(rim, 0.001f), light.angleAttenuation);
        
    const float cosTheta = dot(dirToCamera, Li);
    attenuation *= PhaseFunction(cosTheta, u_pushConstants.anisotropy);
        
    return light.color * light.intensity * attenuation;
}

int GetSpotLightBufferIndex(int i, uint2 tileId)
{
    uint index = tileId.y * u_rendererData.screenTilesCountX + tileId.x;

    uint offset = index * 1024;
    return u_visibleSpotLightIndices[offset + i];
}

float3 CalculateSpotLightContribution(in float3 worldPos, in float3 dirToCamera, in uint2 tileId)
{
    float3 result = 0.f;
    
    for (uint i = 0; i < 1024; i++)
    {
        uint lightIndex = GetSpotLightBufferIndex(i, tileId);
        if (lightIndex == -1)
        {
            break;
        }
        
        result += CalculateSpotLight(u_spotLights[lightIndex], worldPos, dirToCamera);
    }
    
    return result;
}

float CalculateDensity(in float3 worldPos)
{
    float fog = 0.f;
    return max(u_pushConstants.density + fog, 0.f);
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    if (dispatchId.x >= VOXEL_GRID_SIZE_X && dispatchId.y >= VOXEL_GRID_SIZE_Y && dispatchId.z >= VOXEL_GRID_SIZE_Z)
    {
        return;
    }
    
    const float jitter = 0.f;
    const float3 worldPos = GetWorldPositionFromIndex_Jitter(dispatchId, jitter, u_cameraData.nearPlane, u_cameraData.farPlane, u_cameraData.inverseNonReverseViewProj);
    const float3 wo = normalize(u_cameraData.position.xyz - worldPos);
    
    uint2 tileId = (dispatchId.xy / 16) * u_pushConstants.sizeMultiplier.xy;
    
    float3 lighting = u_pushConstants.globalDensity * u_pushConstants.globalColor.xyz;
    lighting += CalculateDirectionalLightContribution(worldPos, wo);
    lighting += CalculatePointLightContribution(worldPos, wo, tileId);
    lighting += CalculateSpotLightContribution(worldPos, wo, tileId);
    
    float4 colorAndDensity = float4(lighting * u_pushConstants.density, u_pushConstants.density);
    
    o_resultTexture[dispatchId] = colorAndDensity;
}