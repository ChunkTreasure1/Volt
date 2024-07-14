#pragma once

#include "PBRHelpers.hlsli"
#include "Structures.hlsli"
#include "Resources.hlsli"
#include "ShadowMapping.hlsli"
#include "Lights.hlsli"


struct PBRConstants
{
    UniformBuffer<ViewData> viewData;
    
    UniformTypedBuffer<DirectionalLight> directionalLight;
    UniformTypedBuffer<PointLight> pointLights;
    UniformTypedBuffer<SpotLight> spotLights;

    UniformTypedBuffer<int> visiblePointLights;
    UniformTypedBuffer<int> visibleSpotLights;
    
    TextureSampler linearSampler;
    TextureSampler pointLinearClampSampler;
    TextureSampler shadowSampler;
    
    UniformTexture<float2> BRDFLuT;
    UniformTexture<float3> environmentIrradiance;
    UniformTexture<float3> environmentRadiance;
    //UniformTexture<float> directionalShadowMap;
};

struct PBRInput
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
    float3 emissive;
    
    float3 worldPosition;
    uint2 tileId;
};

struct LightOutput
{
    float3 diffuse;
    float3 specular;
};

static PBRInput m_pbrInput;
static PBRConstants m_pbrConstants;
static ViewData m_viewData;

static TextureSampler m_shadowSampler;

float3 CalculateDiffuse(in float3 F)
{
    const float3 kd = (1.f - F) * (1.f - m_pbrInput.metallic);
    const float3 diffuseBRDF = kd * m_pbrInput.albedo.xyz;
    
    return diffuseBRDF;
}

float3 CalculateSpecular(float cosLi, float NdotV, in float3 F, float D, float G)
{
    float3 specularBRDF = (F * D * G) / max(EPSILON, 4.f * cosLi * NdotV);
    specularBRDF = clamp(specularBRDF, 0.f, 10.f);
    
    return specularBRDF;
}

LightOutput CalculateSkyAmbiance(in float3 dirToCamera, in float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrInput.normal, dirToCamera), EPSILON);
    const float3 irradiance = m_pbrConstants.environmentIrradiance.SampleLevelCube(m_pbrConstants.linearSampler, m_pbrInput.normal, 0.f);
    
    const float3 F = FresnelSchlickRoughness(baseReflectivity, NdotV, m_pbrInput.roughness);
    const float3 kD = lerp(1.f - F, 0.f, m_pbrInput.metallic);
    
    const float3 diffuseIBL = kD * m_pbrInput.albedo.rgb * irradiance;
    
    uint radianceTextureLevels;
    uint width, height;
    m_pbrConstants.environmentRadiance.GetDimensionsCube(0, width, height, radianceTextureLevels);
    
    const float3 R = 2.f * NdotV * m_pbrInput.normal - dirToCamera;
    const float3 specularIrradiance = m_pbrConstants.environmentRadiance.SampleLevelCube(m_pbrConstants.linearSampler, R, m_pbrInput.roughness * radianceTextureLevels);

    const float2 BRDF = m_pbrConstants.BRDFLuT.SampleLevel2D(m_pbrConstants.pointLinearClampSampler, float2(NdotV, m_pbrInput.roughness), 0);
    const float3 specularIBL = (baseReflectivity * BRDF.x + BRDF.y) * specularIrradiance;
    
    LightOutput output;
    output.diffuse = diffuseIBL;
    output.specular = specularIBL;
    
    return output;
}

float CalculateDirectionalShadow(in DirectionalLight light)
{
    //const uint cascadeIndex = GetCascadeIndexFromWorldPosition(light, m_pbrInput.worldPosition, m_viewData.view);
    //const float3 shadowMapCoords = GetShadowMapCoords(light.viewProjections[cascadeIndex], m_pbrInput.worldPosition);
    //const float result = CalculateDirectionalShadow_Hard(light, m_shadowSampler, m_pbrConstants.directionalShadowMap, m_pbrInput.normal, cascadeIndex, shadowMapCoords);
    return 0.f;
}

LightOutput CalculateDirectionalLight(in DirectionalLight light, in float3 dirToCamera, in float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrInput.normal, dirToCamera), EPSILON);
    
    const float3 Li = normalize(light.direction.xyz);
    const float3 Lradiance = light.color * max(light.intensity, 0.f);
    const float3 Lh = normalize(Li + dirToCamera);
    
    const float cosLi = max(0.f, dot(m_pbrInput.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrInput.normal, Lh));

    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrInput.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrInput.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrInput.roughness);
    
    const float3 diffuseBRDF = CalculateDiffuse(F);
    const float3 specularBRDF = CalculateSpecular(cosLi, NdotV, F, D, G);
    
    float shadow = 1.f;
    //if (light.castShadows)
    //{
    //    shadow = CalculateDirectionalShadow(light);
    //}

    LightOutput output;
    output.diffuse = diffuseBRDF * Lradiance * cosLi * shadow;
    output.specular = specularBRDF * Lradiance * cosLi * shadow;

    return output;
}

LightOutput CalculatePointLight(in PointLight light, float3 dirToCamera, float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrInput.normal, dirToCamera), EPSILON);
    
    const float3 Li = normalize(light.position - m_pbrInput.worldPosition);
    const float lightDistance = length(light.position - m_pbrInput.worldPosition);
    const float3 Lh = normalize(Li + dirToCamera);
    
    const float radius = max(light.radius, 0.f);
    const float falloff = max(light.falloff, 0.f);
    
    float attenuation = clamp(1.f - (lightDistance * lightDistance) / (radius * radius), 0.f, 1.f);
    attenuation *= lerp(attenuation, 1.f, falloff);

    const float3 Lradiance = light.color * max(light.intensity, 0.f) * attenuation;
    
    const float cosLi = max(0.f, dot(m_pbrInput.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrInput.normal, Lh));

    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrInput.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrInput.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrInput.roughness);
    
    const float3 diffureBRDF = CalculateDiffuse(F);
    const float3 specularBRDF = CalculateSpecular(cosLi, NdotV, F, D, G);
    
    LightOutput output;
    output.diffuse = diffureBRDF * Lradiance * cosLi;
    output.specular = diffureBRDF * Lradiance * cosLi;

    return output;
}

LightOutput CalculatePointLights(float3 dirToCamera, float3 baseReflectivity)
{
    LightOutput output;
    output.diffuse = 0.f;
    output.specular = 0.f;

    for (int i = 0; i < MAX_LIGHTS_PER_TILE; i++)
    {
        int lightIndex = GetLightBufferIndex(m_pbrConstants.visiblePointLights, m_viewData.tileCountX, i, m_pbrInput.tileId);
        if (lightIndex == -1)
        {
            break;
        }

        LightOutput result = CalculatePointLight(m_pbrConstants.pointLights.Load(i), dirToCamera, baseReflectivity);
        output.diffuse += result.diffuse;
        output.specular += result.specular;
    }
    
    return output;
}

LightOutput CalculateSpotLight(in SpotLight light, float3 dirToCamera, float3 baseReflectivity)
{
    const float3 Li = normalize(light.position - m_pbrInput.worldPosition);
    const float lightDistance = length(light.position - m_pbrInput.worldPosition);
    const float NdotV = max(dot(m_pbrInput.normal, dirToCamera), EPSILON);
    
    const float cutoff = cos(light.angle * 0.5f);
    const float scos = max(dot(Li, light.direction), cutoff);
    const float rim = (1.f - scos) / (1.f - cutoff);
    
    float attenuation = clamp(1.f - (lightDistance * lightDistance) / (light.range * light.range), 0.f, 1.f);
    attenuation *= lerp(attenuation, 1.f, light.falloff);
    attenuation *= 1.f - pow(max(rim, 0.001f), light.angleAttenuation);
    
    const float3 Lradiance = light.color * max(light.intensity, 0.f) * attenuation;
    const float3 Lh = normalize(Li + dirToCamera);

    const float cosLi = max(0.f, dot(m_pbrInput.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrInput.normal, Lh));

    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrInput.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrInput.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrInput.roughness);
    
    const float3 diffuseBRDF = CalculateDiffuse(F);
    const float3 specularBRDF = CalculateSpecular(cosLi, NdotV, F, D, G);
    
    LightOutput output;
    output.diffuse = diffuseBRDF * Lradiance * cosLi;
    output.specular = diffuseBRDF * Lradiance * cosLi;

    return output;
}

LightOutput CalculateSpotLights(float3 dirToCamera, float3 baseReflectivity, uint spotLightCount)
{
    LightOutput output;
    output.diffuse = 0.f;
    output.specular = 0.f;
    
    for (uint i = 0; i < spotLightCount; i++)
    {
        LightOutput result = CalculateSpotLight(m_pbrConstants.spotLights.Load(0), dirToCamera, baseReflectivity);
        output.diffuse += result.diffuse;
        output.specular += result.specular;
    }
    
    return output;
}

float3 CalculatePBR(in PBRInput input, in PBRConstants constants)
{
    m_pbrInput = input;
    m_pbrConstants = constants;
    
    m_viewData = constants.viewData.Load();
    m_shadowSampler = constants.shadowSampler;    

    const float3 dirToCamera = normalize(m_viewData.cameraPosition.xyz - m_pbrInput.worldPosition);
    const float3 baseReflectivity = lerp(m_dielectricBase, m_pbrInput.albedo.xyz, m_pbrInput.metallic);
    
    LightOutput lightOutput;
    lightOutput.diffuse = 0.f;
    lightOutput.specular = 0.f;
    
    // Skylight
    {
        LightOutput result = CalculateSkyAmbiance(dirToCamera, baseReflectivity);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    // Directional Light
    {
        LightOutput result = CalculateDirectionalLight(constants.directionalLight.Load(0), dirToCamera, baseReflectivity);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    // Point lights
    {
        LightOutput result = CalculatePointLights(dirToCamera, baseReflectivity);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    // Spot lights
    {
        LightOutput result = CalculateSpotLights(dirToCamera, baseReflectivity, m_viewData.spotLightCount);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    const float3 compositeLighting = lightOutput.diffuse + lightOutput.specular + m_pbrInput.emissive;
    return compositeLighting;
}