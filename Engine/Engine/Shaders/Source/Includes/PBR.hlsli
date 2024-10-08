#pragma once

#include "PBRHelpers.hlsli"
#include "Structures.hlsli"
#include "Resources.hlsli"
#include "ShadowMapping.hlsli"
#include "Lights.hlsli"


struct PBRConstants
{
    vt::UniformBuffer<ViewData> viewData;
    
    vt::UniformBuffer<DirectionalLight> directionalLight;
    vt::TypedBuffer<PointLight> pointLights;
    vt::TypedBuffer<SpotLight> spotLights;

    vt::TypedBuffer<int> visiblePointLights;
    vt::TypedBuffer<int> visibleSpotLights;
    
    vt::TextureSampler linearSampler;
    vt::TextureSampler pointLinearClampSampler;
    vt::TextureSampler shadowSampler;
    
    vt::Tex2D<float2> BRDFLuT;
    vt::TexCube<float3> environmentIrradiance;
    vt::TexCube<float3> environmentRadiance;
    vt::Tex2DArray<float> directionalShadowMap;
};

struct PBRInput
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
    float3 emissive;
    float ao;
    
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

static vt::TextureSampler m_shadowSampler;

float3 CalculateDiffuse(in float3 F)
{
    const float3 kd = (1.f - F) * (1.f - m_pbrInput.metallic);
    const float3 diffuseBRDF = kd * m_pbrInput.albedo.xyz / PI;
    
    return diffuseBRDF;
}

float3 CalculateSpecular(float cosLi, float NdotV, in float3 F, float D, float G)
{
    float3 specularBRDF = (F * D * G) / max(EPSILON, 4.f * cosLi * NdotV);
    specularBRDF = clamp(specularBRDF, 0.f, 10.f);
    
    return specularBRDF;
}

float3 CalculateSkyAmbiance(in float3 dirToCamera, in float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrInput.normal, dirToCamera), 0.f);

    const float3 F = FresnelSchlickRoughness(baseReflectivity, NdotV, m_pbrInput.roughness);
    const float3 kD = (1.f - F) * (1.f - m_pbrInput.metallic);

    const float3 irradiance = m_pbrConstants.environmentIrradiance.SampleLevel(m_pbrConstants.linearSampler, m_pbrInput.normal, 0.f);
    const float3 diffuse = irradiance * m_pbrInput.albedo.xyz;

    // #TODO_Ivar: This is quite slow
    uint radianceTextureLevels;
    uint width, height;
    m_pbrConstants.environmentRadiance.GetDimensions(0, width, height, radianceTextureLevels);
    
    const float3 R = reflect(-dirToCamera, m_pbrInput.normal);
    const float3 specularIrradiance = m_pbrConstants.environmentRadiance.SampleLevel(m_pbrConstants.linearSampler, R, m_pbrInput.roughness * radianceTextureLevels); 
    const float2 BRDF = m_pbrConstants.BRDFLuT.SampleLevel(m_pbrConstants.pointLinearClampSampler, float2(NdotV, m_pbrInput.roughness), 0);
    const float3 specular = specularIrradiance * (F * BRDF.x + BRDF.y); 
    
    return kD * diffuse + specular;
}

float CalculateDirectionalShadow(in DirectionalLight light)
{
    const uint cascadeIndex = GetCascadeIndexFromWorldPosition(light, m_pbrInput.worldPosition, m_viewData.view);
    const float3 shadowMapCoords = GetShadowMapCoords(light.viewProjections[cascadeIndex], m_pbrInput.worldPosition);
    const float result = CalculateDirectionalShadow_Hard(light, m_shadowSampler, m_pbrConstants.directionalShadowMap, m_pbrInput.normal, cascadeIndex, shadowMapCoords);
    return result;
}

float3 CalculateDirectionalLight(in DirectionalLight light, in float3 dirToCamera, in float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrInput.normal, dirToCamera), EPSILON);
    
    const float3 Li = normalize(light.direction.xyz);
    const float3 Lradiance = light.color * max(light.intensity, 0.f);
    const float3 Lh = normalize(Li + dirToCamera);
    
    const float cosLi = max(0.f, dot(m_pbrInput.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrInput.normal, Lh));

    const float3 F = FresnelSchlick(baseReflectivity, max(0.f, dot(Lh, dirToCamera)));
    const float D = DistributionGGX(cosLh * cosLh, m_pbrInput.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrInput.roughness);
    
    const float3 diffuseBRDF = CalculateDiffuse(F);
    const float3 specularBRDF = CalculateSpecular(cosLi, NdotV, F, D, G);
    
    float shadow = 1.f;
    if (light.castShadows)
    {
        shadow = CalculateDirectionalShadow(light);
    }

    return (diffuseBRDF + specularBRDF) * Lradiance * cosLi * shadow;
}

float3 CalculatePointLight(in PointLight light, float3 dirToCamera, float3 baseReflectivity)
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

    const float3 F = FresnelSchlick(baseReflectivity, max(0.f, dot(Lh, dirToCamera)));
    const float D = DistributionGGX(cosLh * cosLh, m_pbrInput.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrInput.roughness);
    
    const float3 diffuseBRDF = CalculateDiffuse(F);
    const float3 specularBRDF = CalculateSpecular(cosLi, NdotV, F, D, G);
    
    return (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
}

float3 CalculatePointLights(float3 dirToCamera, float3 baseReflectivity, uint pointLightCount)
{
    float3 output = 0.f;

    for (int i = 0; i < pointLightCount; i++)
    {
        int lightIndex = GetLightBufferIndex(m_pbrConstants.visiblePointLights, m_viewData.tileCountX, i, m_pbrInput.tileId);
        if (lightIndex == -1)
        {
            break;
        }

        output += CalculatePointLight(m_pbrConstants.pointLights.Load(i), dirToCamera, baseReflectivity);
    }
    
    return output;
}

float3 CalculateSpotLight(in SpotLight light, float3 dirToCamera, float3 baseReflectivity)
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
    
    return (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
}

float3 CalculateSpotLights(float3 dirToCamera, float3 baseReflectivity, uint spotLightCount)
{
    float3 output = 0.f;
    for (uint i = 0; i < spotLightCount; i++)
    {
        output += CalculateSpotLight(m_pbrConstants.spotLights.Load(i), dirToCamera, baseReflectivity);
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
    
    float3 lightOutput = 0.f;
     
    // Skylight
    {
        lightOutput += CalculateSkyAmbiance(dirToCamera, baseReflectivity) * input.ao; 
    }
    
    // Directional Light
    {
        lightOutput += CalculateDirectionalLight(constants.directionalLight.Load(), dirToCamera, baseReflectivity);
    }
    
    // Point lights
    {
        lightOutput += CalculatePointLights(dirToCamera, baseReflectivity, m_viewData.pointLightCount);
    }
    
    // Spot lights
    {
        lightOutput += CalculateSpotLights(dirToCamera, baseReflectivity, m_viewData.spotLightCount);
    }
    
    const float3 compositeLighting = lightOutput + m_pbrInput.emissive;
    return compositeLighting;
}