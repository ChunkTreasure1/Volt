#pragma once

#include "PBRHelpers.hlsli"
#include "Structures.hlsli"
#include "Resources.hlsli"

struct PBRConstants
{
    TypedBuffer<ViewData> viewData;
    
    TypedBuffer<DirectionalLight> DirectionalLight;
    
    TextureSampler linearSampler;
    TextureSampler pointLinearClampSampler;
    
    TTexture<float2> BRDFLuT;
    TTexture<float3> environmentIrradiance;
    TTexture<float3> environmentRadiance;
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
static PBRConstants m_pbrConstants;

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
    const float3 irradiance = m_pbrConstants.environmentIrradiance.SampleLevelCube(m_pbrConstants.linearSampler.Get(), m_pbrInput.normal, 0.f);
    
    const float3 F = FresnelSchlickRoughness(baseReflectivity, NdotV, m_pbrInput.roughness);
    const float3 kD = lerp(1.f - F, 0.f, m_pbrInput.metallic);
    
    const float3 diffuseIBL = kD * m_pbrInput.albedo.rgb * irradiance;
    
    uint radianceTextureLevels;
    uint width, height;
    m_pbrConstants.environmentRadiance.GetDimensionsCube(0, width, height, radianceTextureLevels);
    
    const float3 R = 2.f * NdotV * m_pbrInput.normal - dirToCamera;
    const float3 specularIrradiance = m_pbrConstants.environmentRadiance.SampleLevelCube(m_pbrConstants.linearSampler.Get(), R, m_pbrInput.roughness * radianceTextureLevels);

    const float2 BRDF = m_pbrConstants.BRDFLuT.SampleLevel2D(m_pbrConstants.pointLinearClampSampler.Get(), float2(NdotV, m_pbrInput.roughness), 0);
    const float3 specularIBL = (baseReflectivity * BRDF.x + BRDF.y) * specularIrradiance;
    
    LightOutput output;
    output.diffuse = diffuseIBL;
    output.specular = specularIBL;
    
    return output;
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
    m_pbrConstants = constants;
    
    const ViewData viewData = constants.viewData.Load(0);
    
    const float3 dirToCamera = normalize(viewData.cameraPosition.xyz - m_pbrInput.worldPosition);
    const float3 baseReflectivity = lerp(m_dielectricBase, m_pbrInput.albedo.xyz, m_pbrInput.metallic);
    
    LightOutput lightOutput;
    lightOutput.diffuse = 0.f;
    lightOutput.specular = 0.f;
    
    // Directional Light
    {
        LightOutput result = CalculateDirectionalLight(constants.DirectionalLight.Load(0), dirToCamera, baseReflectivity);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    // Skylight
    {
        LightOutput result = CalculateSkyAmbiance(dirToCamera, baseReflectivity);
        lightOutput.diffuse += result.diffuse;
        lightOutput.specular += result.specular;
    }
    
    const float3 compositeLighting = lightOutput.diffuse + lightOutput.specular + m_pbrInput.emissive;
    return compositeLighting;
}