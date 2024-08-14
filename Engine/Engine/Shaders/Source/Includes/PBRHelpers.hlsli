#pragma once

static const float3 m_dielectricBase = 0.04f;
static const float PI = 3.14159265359f;
static const float HALF_PI = 1.57079f;
static const float EPSILON = 0.0001f;

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * (a2 - 1.f) + 1.f;
    denom = PI * denom * denom;

    return a2 / max(denom, EPSILON);
}

// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

float3 FresnelSchlick(float3 baseReflectivity, float cosTheta)
{
    return baseReflectivity + (1.0 - baseReflectivity) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 FresnelSchlickRoughness(float3 baseReflectivity, float cosTheta, float roughness)
{
    return baseReflectivity + (max(1.f - roughness, baseReflectivity) - baseReflectivity) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

float4 CalculateLuminanceFromLight(in float3 accumulatedLight)
{
    float4 result;

    const float brightness = dot(accumulatedLight, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.f)
    {
        result = float4(accumulatedLight, 1.f);
    }
    else
    {
        result = float4(0.f, 0.f, 0.f, 1.f);
    }

    return result;
}