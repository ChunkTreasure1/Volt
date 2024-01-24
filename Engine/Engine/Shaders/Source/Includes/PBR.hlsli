#pragma once

#include "PBRHelpers.hlsli"
#include "Structures.hlsli"
#include "Resources.hlsli"

struct PBRConstants
{
    TypedBuffer<ViewData> viewData;
    
    TypedBuffer<DirectionalLight> DirectionalLight;
};

struct PBRInput
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
    float3 emissive;
    
    float3 worldPosition;
};

struct LightOutput
{
    float3 diffuse;
    float3 specular;
};

static PBRInput m_pbrInput;

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
    
    LightOutput output;
    output.diffuse = diffuseBRDF * Lradiance * cosLi;
    output.specular = specularBRDF * Lradiance * cosLi;

    return output;
}

float3 CalculatePBR(in PBRInput input, in PBRConstants constants)
{
    m_pbrInput = input;
    
    const ViewData viewData = constants.viewData.Load(0);
    
    const float3 dirToCamera = normalize(viewData.cameraPosition.xyz - m_pbrInput.worldPosition);
    const float3 baseReflectivity = lerp(m_dielectricBase, m_pbrInput.albedo.xyz, m_pbrInput.metallic);
    
    LightOutput lightOutput;
    lightOutput.diffuse = 0.f;
    lightOutput.specular = 0.f;
    
    float3 ambient = 0.03f * input.albedo.xyz;
    
    // Directional Light
    {
        LightOutput result = CalculateDirectionalLight(constants.DirectionalLight.Load(0), dirToCamera, baseReflectivity);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    const float3 compositeLighting = lightOutput.diffuse + lightOutput.specular + m_pbrInput.emissive + ambient;
    return compositeLighting;
}