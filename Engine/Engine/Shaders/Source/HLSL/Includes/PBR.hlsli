#include "Buffers.hlsli"
#include "PBRHelpers.hlsli"
#include "SamplerStates.hlsli"
#include "ShadowMapping.hlsli"
#include "Utility.hlsli"

#include "FogCommon.hlsli"

#ifndef PBR_H
#define PBR_H

#define XE_GTAO_OCCLUSION_TERM_SCALE                    (1.5f)      // for packing in UNORM (because raw, pre-denoised occlusion term can overshoot 1 but will later average out to 1)

struct PBRData
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
    float3 emissive;
    bool castAO;
    
    float3 worldPosition;
    
    float4 position;
};

struct LightingOutput
{
    float3 diffuse;
    float3 specular;
};

struct ShaderParameters
{
    float4 position;
};

struct ObjectParameters
{
    float3 worldPosition;
};

struct PBRParamters
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
    float3 emissive;
    float ao;
};

static ShaderParameters m_shaderParameters;
static PBRParamters m_pbrParameters;
static ObjectParameters m_objectParameters;

Texture2D<float2> u_brdfLut : register(BINDING_BRDF, SPACE_PBR_RESOURCES);
TextureCube<float4> u_irradianceTexture : register(BINDING_IRRADIANCE, SPACE_PBR_RESOURCES);
TextureCube<float4> u_radianceTexture : register(BINDING_RADIANCE, SPACE_PBR_RESOURCES);

Texture2DArray u_directionalShadowMap : register(BINDING_DIR_SHADOWMAP, SPACE_PBR_RESOURCES);
Texture2D u_spotlightShadowMaps[8] : register(BINDING_SPOT_SHADOWMAPS, SPACE_PBR_RESOURCES);
TextureCube u_pointLightShadowMaps[8] : register(BINDING_POINT_SHADOWMAPS, SPACE_PBR_RESOURCES);

Texture3D u_volumetricFogTexture : register(BINDING_VOLUMETRIC_FOG_TEXTURE, SPACE_PBR_RESOURCES);
Texture2D<uint> u_aoTexture : register(BINDING_AO_TEXTURE, SPACE_PBR_RESOURCES);

float3 CalculateLightDiffuse(in float3 F)
{
    const float3 kd = (1.f - F) * (1.f - m_pbrParameters.metallic);
    const float3 diffuseBRDF = kd * m_pbrParameters.albedo.xyz;

    return diffuseBRDF;
}

float3 CalculateLightSpecular(float cosLi, float NdotV, in float3 F, float D, float G)
{
    float3 specularBRDF = (F * D * G) / max(EPSILON, 4.f * cosLi * NdotV);
    specularBRDF = clamp(specularBRDF, 0.f, 10.f);
    
    return specularBRDF;
}

///// Directional Light //////
float CalculateDirectionalShadow(in DirectionalLight light)
{
    const uint cascadeIndex = GetCascadeIndexFromWorldPosition(light, m_objectParameters.worldPosition, u_cameraData.view);
    const float3 shadowMapCoords = GetShadowMapCoords(light.viewProjections[cascadeIndex], m_objectParameters.worldPosition);
    const float result = (light.softShadows == 1) ? CalculateDirectionalShadow_Soft(light, u_directionalShadowMap, m_pbrParameters.normal, cascadeIndex, shadowMapCoords) : CalculateDirectionalShadow_Hard(light, u_directionalShadowMap, m_pbrParameters.normal, cascadeIndex, shadowMapCoords);
    return result;
}

LightingOutput CalculateDirectionalLight(in DirectionalLight light, in float3 dirToCamera, in float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);

    const float3 Li = normalize(light.direction.xyz);
    const float3 Lradiance = light.colorIntensity.xyz * max(light.colorIntensity.w, 0.f);
    const float3 Lh = normalize(Li + dirToCamera);

    const float cosLi = max(0.f, dot(m_pbrParameters.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));

    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrParameters.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);

    const float3 diffuseBRDF = CalculateLightDiffuse(F);
    const float3 specularBRDF = CalculateLightSpecular(cosLi, NdotV, F, D, G);

    float shadow = 1.f;
    if (light.castShadows == 1)
    {
        shadow = CalculateDirectionalShadow(light);
    }

    LightingOutput output;
    output.diffuse = diffuseBRDF * Lradiance * cosLi * shadow;
    output.specular = specularBRDF * Lradiance * cosLi * shadow;

    return output;
}
//////////////////////////////

///// Point Light //////
LightingOutput CalculatePointLight(in PointLight light, float3 dirToCamera, float3 baseReflectivity)
{
    const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);

    const float3 Li = normalize(light.position.xyz - m_objectParameters.worldPosition);
    const float lightDistance = length(light.position.xyz - m_objectParameters.worldPosition);
    const float3 Lh = normalize(Li + dirToCamera);

    float attenuation = clamp(1.f - (lightDistance * lightDistance) / (light.radius * light.radius), 0.f, 1.f);
    attenuation *= lerp(attenuation, 1.f, light.falloff);

    const float3 Lradiance = light.color.xyz * max(light.intensity, 0.f) * attenuation;

    const float cosLi = max(0.f, dot(m_pbrParameters.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));

    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrParameters.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);

    const float3 diffuseBRDF = CalculateLightDiffuse(F);
    const float3 specularBRDF = CalculateLightSpecular(cosLi, NdotV, F, D, G);

    float shadow = 1.f;
    //if (light.castShadows == 1)
    //{
    //    shadow = CalculatePointLightShadow(light, u_pointLightShadowMaps[light.shadowMapIndex], m_objectParameters.worldPosition);
    //}
    
    LightingOutput output;
    output.diffuse = diffuseBRDF * Lradiance * cosLi * shadow;
    output.specular = specularBRDF * Lradiance * cosLi * shadow;
    
    return output;
}

int GetPointLightBufferIndex(int i, uint2 tileId)
{
    uint index = tileId.y * u_rendererData.screenTilesCountX + tileId.x;

    uint offset = index * 1024;
    return u_visiblePointLightIndices[offset + i];
}

int GetPointLightCount(uint2 tileId)
{
    int result = 0;
    for (int i = 0; i < u_sceneData.pointLightCount; i++)
    {
        uint lightIndex = GetPointLightBufferIndex(i, tileId);
        if (lightIndex == -1)
            break;

        result++;
    }

    return result;
}

LightingOutput CalculatePointLights(float3 dirToCamera, float3 baseReflectivity, uint2 tileId)
{
    LightingOutput lightAccumulation;
    lightAccumulation.diffuse = 0.f;
    lightAccumulation.specular = 0.f;
    
#if 1
    for (uint i = 0; i < 1024; i++)
    {
        uint lightIndex = GetPointLightBufferIndex(i, tileId);
        if (lightIndex == -1)
        {
            break;
        }
    
        LightingOutput result = CalculatePointLight(u_pointLights[lightIndex], dirToCamera, baseReflectivity);
        lightAccumulation.diffuse += result.diffuse;
        lightAccumulation.specular += result.specular;
    }
#else    
    for (uint i = 0; i < u_sceneData.pointLightCount; i++)
    {
        LightingOutput result = CalculatePointLight(u_pointLights[i], dirToCamera, baseReflectivity);
        lightAccumulation.diffuse += result.diffuse;
        lightAccumulation.specular += result.specular;
    }
#endif

    return lightAccumulation;
}
////////////////////////

///// Spot Lights /////
LightingOutput CalculateSpotLight(in SpotLight light, float3 dirToCamera, float3 baseReflectivity)
{
    const float3 Li = normalize(light.position.xyz - m_objectParameters.worldPosition);
    const float lightDistance = length(light.position.xyz - m_objectParameters.worldPosition);
    const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);
    
    const float cutoff = cos(light.angle * 0.5f);
    const float scos = max(dot(Li, light.direction), cutoff);
    const float rim = (1.f - scos) / (1.f - cutoff);
    
    float attenuation = clamp(1.f - (lightDistance * lightDistance) / (light.range * light.range), 0.f, 1.f);
    attenuation *= lerp(attenuation, 1.f, light.falloff);
    attenuation *= 1.f - pow(max(rim, 0.001f), light.angleAttenuation);

    const float3 Lradiance = light.color * max(light.intensity, 0.f) * attenuation;
    const float3 Lh = normalize(Li + dirToCamera);
    
    const float cosLi = max(0.f, dot(m_pbrParameters.normal, Li));
    const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));

    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrParameters.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);

    const float3 diffuseBRDF = CalculateLightDiffuse(F);
    const float3 specularBRDF = CalculateLightSpecular(cosLi, NdotV, F, D, G);
    
    float shadow = 1.f;
    //if (light.castShadows == 1)
    //{
    //    shadow = CalculateSpotLightShadow(light, u_spotlightShadowMaps[light.shadowMapIndex], m_objectParameters.worldPosition);
    //}
    
    LightingOutput output;
    output.diffuse = diffuseBRDF * Lradiance * cosLi * shadow;
    output.specular = specularBRDF * Lradiance * cosLi * shadow;
    
    return output;
}

int GetSpotLightBufferIndex(int i, uint2 tileId)
{
    uint index = tileId.y * u_rendererData.screenTilesCountX + tileId.x;

    uint offset = index * 1024;
    return u_visibleSpotLightIndices[offset + i];
}

int GetSpotLightCount(uint2 tileId)
{
    int result = 0;
    for (int i = 0; i < u_sceneData.spotLightCount; i++)
    {
        uint lightIndex = GetSpotLightBufferIndex(i, tileId);
        if (lightIndex == -1)
            break;

        result++;
    }

    return result;
}

LightingOutput CalculateSpotLights(float3 dirToCamera, float3 baseReflectivity, uint2 tileId)
{
    LightingOutput lightAccumulation;
    lightAccumulation.diffuse = 0.f;
    lightAccumulation.specular = 0.f;
    
#if 1
    for (uint i = 0; i < 1024; i++)
    {
        uint lightIndex = GetSpotLightBufferIndex(i, tileId);
        if (lightIndex == -1)
        {
            break;
        }
        
        LightingOutput output = CalculateSpotLight(u_spotLights[lightIndex], dirToCamera, baseReflectivity);
        lightAccumulation.diffuse += output.diffuse;
        lightAccumulation.specular += output.specular;
    }
#else
    for (uint i = 0; i < u_sceneData.spotLightCount; i++)
    {
        LightingOutput output = CalculateSpotLight(u_spotLights[i], dirToCamera, baseReflectivity);
    
        lightAccumulation.diffuse += output.diffuse;
        lightAccumulation.specular += output.specular;
    }

#endif
    
        return lightAccumulation;
}
///////////////////////

float3 CalculateAreaDiffuse(float3 dirToCamera, float3 baseReflectivity, float3 Lh)
{
    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
    return CalculateLightDiffuse(F);
}

float3 CalculateSpecular_Sphere(in SphereLight light, float distanceToLight, float3 dirToCamera, float3 baseReflectivity, float3 Lh, float3 Li)
{
    const float cosLi = clamp(dot(m_pbrParameters.normal, Li), -0.999f, 0.999f);
    const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));
    const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);
    
    ///// Distribution calculation //////
    float a = m_pbrParameters.roughness * m_pbrParameters.roughness;
    const float alphaPrime = saturate(light.radius / (distanceToLight * 2.f) + a);
    
    const float a2 = alphaPrime * a;
    float denom = cosLh * cosLh * (a2 - 1.f) + 1.f;
    denom = PI * denom * denom;

    const float D = a2 / max(denom, EPSILON);
    /////////////////////////////////////
    
    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);

    return CalculateLightSpecular(cosLi, NdotV, F, D, G);
}

float3 CalculateSpecularArea(float3 dirToCamera, float3 baseReflectivity, float3 Lh, float3 Li)
{
    const float cosLi = clamp(dot(m_pbrParameters.normal, Li), -0.999f, 0.999f);
    const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));
    const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);
    
    const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
    const float D = DistributionGGX(cosLh * cosLh, m_pbrParameters.roughness);
    const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);
    return CalculateLightSpecular(cosLi, NdotV, F, D, G);
}

float CalculateIlluminanceOnSpereOrDisk(float cosLi, float sinSigmaSqr)
{
    float sinTheta = sqrt(1.f - cosLi * cosLi);
    float illuminance = 0.f;
    
    // Test from page 38 of https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
    if (cosLi * cosLi > sinSigmaSqr)
    {
        illuminance = PI * sinSigmaSqr * saturate(cosLi);
    }
    else
    {
        const float x = sqrt(1.f / sinSigmaSqr - 1.f);
        const float y = -x * (cosLi / sinTheta);
        const float sinThetaSqrY = sinTheta * sqrt(1.f - y * y);
        illuminance = (cosLi * acos(y) - x * sinThetaSqrY) * sinSigmaSqr + atan(sinThetaSqrY) / x;
    }
    
    return max(illuminance, 0.f);
}

float3 GetSpecularDominantDirectionalArea(float3 reflected)
{
    const float lerpFactor = 1.f - m_pbrParameters.roughness;
    return normalize(lerp(m_pbrParameters.normal, reflected, lerpFactor));
}

LightingOutput CalculateSphereLight(in SphereLight light, float3 dirToCamera, float3 baseReflectivity)
{
    const float3 Lunnorm = light.position.xyz - m_objectParameters.worldPosition;
    const float lightDistance = length(light.position.xyz - m_objectParameters.worldPosition);
    const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);
    
    const float3 Li = normalize(light.position.xyz - m_objectParameters.worldPosition);
    
    const float cosLi = clamp(dot(m_pbrParameters.normal, Li), -0.999f, 0.999f);
    
    const float radiusSqr = light.radius * light.radius;
    const float sinSigmaSqr = min(radiusSqr / (lightDistance * lightDistance), 0.9999f);

    const float illuminance = CalculateIlluminanceOnSpereOrDisk(cosLi, sinSigmaSqr);
    
    // Specular
    float3 reflected = reflect(-dirToCamera, m_pbrParameters.normal);
    //reflected = GetSpecularDominantDirectionalArea(reflected); // Causes tearing
    
    const float3 centerToRay = dot(Lunnorm, reflected) * reflected - Lunnorm;
    const float3 closestPoint = Lunnorm + centerToRay * saturate(light.radius / length(centerToRay));
    
    const float3 Lispec = normalize(closestPoint);
    const float distanceSpec = length(closestPoint);
    
    const float3 Lhspec = normalize(Lispec + dirToCamera);
    const float3 Lhdiff = normalize(Li + dirToCamera);

    const float3 diffuseBRDF = CalculateAreaDiffuse(dirToCamera, baseReflectivity, Lhdiff);
    const float3 specularBRDF = CalculateSpecular_Sphere(light, distanceSpec, dirToCamera, baseReflectivity, Lhspec, Lispec);

    const float3 Lradiance = light.color * light.intensity;
    
    LightingOutput output;
    output.diffuse = diffuseBRDF * illuminance * Lradiance;
    output.specular = specularBRDF * illuminance * Lradiance;

    return output;
}

LightingOutput CalculateSphereLights(float3 dirToCamera, float3 baseReflectivity)
{
    LightingOutput lightAccumulation;
    lightAccumulation.diffuse = 0.f;
    lightAccumulation.specular = 0.f;
    
    for (uint i = 0; i < u_sceneData.sphereLightCount; i++)
    {
        LightingOutput result = CalculateSphereLight(u_sphereLights[i], dirToCamera, baseReflectivity);
        lightAccumulation.diffuse += result.diffuse;
        lightAccumulation.specular += result.specular;
    }

    return lightAccumulation;
}

///// Rectangle light

float RightPyramidSolidAngle(float distance, float halfWidth, float halfHeight)
{
    float a = halfWidth;
    float b = halfHeight;
    float h = distance;
    
    return 4.f * asin(a * b / sqrt(a * a + h * h) * (b * b + h * h));
}

float RectangleSolidAngle(float3 worldPos, float3 p0, float3 p1, float3 p2, float3 p3)
{
    const float3 v0 = p0 - worldPos;
    const float3 v1 = p1 - worldPos;
    const float3 v2 = p2 - worldPos;
    const float3 v3 = p3 - worldPos;
    
    const float3 n0 = normalize(cross(v0, v1));
    const float3 n1 = normalize(cross(v1, v2));
    const float3 n2 = normalize(cross(v2, v3));
    const float3 n3 = normalize(cross(v3, v0));

    const float g0 = acos(dot(-n0, n1));
    const float g1 = acos(dot(-n1, n2));
    const float g2 = acos(dot(-n2, n3));
    const float g3 = acos(dot(-n3, n0));

    return g0 + g1 + g2 + g3 - 2.f * PI;
}

// From Wicked Engine

// o		: ray origin
// d		: ray direction
// returns distance on the ray to the object if hit, 0 otherwise
float Trace_plane(float3 o, float3 d, float3 planeOrigin, float3 planeNormal)
{
    return dot(planeNormal, (planeOrigin - o) / dot(planeNormal, d));
}

// o		: ray origin
// d		: ray direction
// A,B,C	: traingle corners
// returns distance on the ray to the object if hit, 0 otherwise
float Trace_triangle(float3 o, float3 d, float3 A, float3 B, float3 C)
{
    float3 planeNormal = normalize(cross(B - A, C - B));
    float t = Trace_plane(o, d, A, planeNormal);
    float3 p = o + d * t;

    float3 N1 = normalize(cross(B - A, p - B));
    float3 N2 = normalize(cross(C - B, p - C));
    float3 N3 = normalize(cross(A - C, p - A));

    float d0 = dot(N1, N2);
    float d1 = dot(N2, N3);

    float threshold = 1.0f - 0.001f;
    return (d0 > threshold && d1 > threshold) ? 1.0f : 0.0f;
}

// o		: ray origin
// d		: ray direction
// A,B,C,D	: rectangle corners
// returns distance on the ray to the object if hit, 0 otherwise
float Trace_rectangle(float3 o, float3 d, float3 A, float3 B, float3 C, float3 D)
{
    return max(Trace_triangle(o, d, A, B, C), Trace_triangle(o, d, C, D, A));
}

// Return the closest point on the segment (with limit) 
float3 ClosestPointOnSegment(float3 a, float3 b, float3 c)
{
    float3 ab = b - a;
    float t = dot(c - a, ab) / dot(ab, ab);
    return a + saturate(t) * ab;
}

LightingOutput CalculateRectangleLight(in RectangleLight light, float3 dirToCamera, float3 baseReflectivity)
{
    const float3 Li = normalize(light.position.xyz - m_objectParameters.worldPosition);
    const float lightDistance = length(light.position.xyz - m_objectParameters.worldPosition);
 
    float illuminance = 0.f;
    
    const float halfWidth = light.width * 0.5f;
    const float halfHeight = light.height * 0.5f;
        
    const float3 p0 = light.position + light.left * -halfWidth + light.up * halfHeight;
    const float3 p1 = light.position + light.left * -halfWidth + light.up * -halfHeight;
    const float3 p2 = light.position + light.left * halfWidth + light.up * -halfHeight;
    const float3 p3 = light.position + light.left * halfWidth + light.up * halfHeight;
    
    if (dot(m_objectParameters.worldPosition - light.position, light.direction) > 0.f)
    {
        const float solidAngle = RectangleSolidAngle(m_objectParameters.worldPosition, p0, p1, p2, p3);
        
        illuminance = solidAngle * 0.2f *
        (
            saturate(dot(normalize(p0 - m_objectParameters.worldPosition), m_pbrParameters.normal) +
            saturate(dot(normalize(p1 - m_objectParameters.worldPosition), m_pbrParameters.normal)) +
            saturate(dot(normalize(p2 - m_objectParameters.worldPosition), m_pbrParameters.normal)) +
            saturate(dot(normalize(p3 - m_objectParameters.worldPosition), m_pbrParameters.normal)) +
            saturate(dot(normalize(light.position - m_objectParameters.worldPosition), m_pbrParameters.normal)))
        );
    }
    
    illuminance = max(0.f, illuminance);
    
    // Specular
    float3 reflected = reflect(-dirToCamera, m_pbrParameters.normal);
    reflected = GetSpecularDominantDirectionalArea(reflected);
    
    float traced = Trace_rectangle(m_objectParameters.worldPosition, reflected, p0, p1, p2, p3);
    
    float3 Lispec = Li;
    
    if (traced > 0.f)
    {
        Lispec = reflected;
    }
    else
    {
        // The trace didn't succeed, so we need to find the closest point to the ray on the rectangle

			// We find the intersection point on the plane of the rectangle
        float3 tracedPlane = m_objectParameters.worldPosition + reflected * Trace_plane(m_objectParameters.worldPosition, reflected, light.position, light.direction);

			// Then find the closest point along the edges of the rectangle (edge = segment)
        float3 PC[4] =
        {
            ClosestPointOnSegment(p0, p1, tracedPlane),
				ClosestPointOnSegment(p1, p2, tracedPlane),
				ClosestPointOnSegment(p2, p3, tracedPlane),
				ClosestPointOnSegment(p3, p0, tracedPlane),
        };
        float dist[4] =
        {
            distance(PC[0], tracedPlane),
				distance(PC[1], tracedPlane),
				distance(PC[2], tracedPlane),
				distance(PC[3], tracedPlane),
        };

        float3 min = PC[0];
        float minDist = dist[0];
        
        [unroll]
        for (uint iLoop = 1; iLoop < 4; iLoop++)
        {
            if (dist[iLoop] < minDist)
            {
                minDist = dist[iLoop];
                min = PC[iLoop];
            }
        }

        Lispec = min - m_objectParameters.worldPosition;
    }
    
    Lispec = normalize(Lispec);

    const float3 Lradiance = light.color * light.intensity * illuminance;
    const float cosLispec = clamp(dot(m_pbrParameters.normal, Lispec), -0.999f, 0.999f);
    const float cosLidiff = clamp(dot(m_pbrParameters.normal, Li), -0.999f, 0.999f);
    
    const float3 Lhdiff = normalize(Li + dirToCamera);
    const float3 Lhspec = normalize(Lispec + dirToCamera);
    
    const float3 diffuseBRDF = CalculateAreaDiffuse(dirToCamera, baseReflectivity, Lhdiff);
    const float3 specularBRDF = CalculateSpecularArea(dirToCamera, baseReflectivity, Lhspec, Lispec);
    
    LightingOutput output;
    output.diffuse = max(diffuseBRDF * cosLidiff * Lradiance, 0.f);
    output.diffuse = max(specularBRDF * cosLispec * Lradiance, 0.f);
    
    return output;
}

LightingOutput CalculateRectangleLights(float3 dirToCamera, float3 baseReflectivity)
{
    LightingOutput lightAccumulation;
    lightAccumulation.diffuse = 0.f;
    lightAccumulation.specular = 0.f;
    
    for (uint i = 0; i < u_sceneData.rectangleLightCount; i++)
    {
        LightingOutput result = CalculateRectangleLight(u_rectangleLights[i], dirToCamera, baseReflectivity);
        lightAccumulation.diffuse += result.diffuse;
        lightAccumulation.specular += result.specular;
    }

    return lightAccumulation;
}

LightingOutput CalculateSkyAmbiance(float3 dirToCamera, float3 baseReflectivity)
{
    const float NdotV = max(0.f, dot(m_pbrParameters.normal, dirToCamera));
    const float3 irradiance = u_irradianceTexture.SampleLevel(u_linearSampler, m_pbrParameters.normal, 0).rgb;

    const float3 F = FresnelSchlickRoughness(baseReflectivity, NdotV, m_pbrParameters.roughness);
    const float3 kD = lerp(1.f - F, 0.f, m_pbrParameters.metallic);
    
    const float3 diffuseIBL = kD * m_pbrParameters.albedo.rgb * irradiance;
    
    uint radianceTextureLevels;
    uint width, height;
    u_radianceTexture.GetDimensions(0, width, height, radianceTextureLevels);
    
    const float3 R = 2.f * NdotV * m_pbrParameters.normal - dirToCamera;
    const float3 specularIrradiance = u_radianceTexture.SampleLevel(u_linearSampler, R, m_pbrParameters.roughness * radianceTextureLevels).rgb;
    
    const float2 brdf = u_brdfLut.SampleLevel(u_pointLinearSamplerClamp, float2(NdotV, m_pbrParameters.roughness), 0).rg;
    const float3 specularIBL = (baseReflectivity * brdf.x + brdf.y) * specularIrradiance;
    
    LightingOutput output;
    output.diffuse = diffuseIBL * u_sceneData.ambianceIntensity;
    output.specular = specularIBL * u_sceneData.ambianceIntensity;
    
    return output;
}

LightingOutput CalculateAmbiance(float3 dirToCamera, float3 baseReflectivity)
{
    return CalculateSkyAmbiance(dirToCamera, baseReflectivity);
}

float CalculateAO(float2 screenPosition)
{
    const float ao = (u_aoTexture.Load(uint3(screenPosition, 0)).x >> 24) / 255.f;
    float finalAO = min(ao * XE_GTAO_OCCLUSION_TERM_SCALE, 1.f);

    return finalAO;
}

float3 CalculateInscatteredLight(in float3 accumulatedLight)
{
    const float3 texCoords = GetTexCoordsFromWorldPosition(m_objectParameters.worldPosition, u_cameraData.nearPlane, u_cameraData.farPlane, mul(u_cameraData.nonReversedProj, u_cameraData.view));
    const float4 scatteredLight = tex3DTricubic(u_volumetricFogTexture, u_linearSampler, texCoords, float3(VOXEL_GRID_SIZE_X, VOXEL_GRID_SIZE_Y, VOXEL_GRID_SIZE_Z));
    const float transmittance = scatteredLight.a;
    
    return accumulatedLight * transmittance + scatteredLight.rgb;
}

#ifndef SSS

float3 CalculatePBR(in PBRData pbrData)

#else

LightingOutput CalculatePBR(in PBRData pbrData)

#endif
{
    m_pbrParameters.albedo = pbrData.albedo;
    m_pbrParameters.normal = pbrData.normal;
    m_pbrParameters.metallic = pbrData.metallic;
    m_pbrParameters.roughness = pbrData.roughness;
    m_pbrParameters.emissive = pbrData.emissive;

    m_objectParameters.worldPosition = pbrData.worldPosition;

    const float3 dirToCamera = normalize(u_cameraData.position.xyz - m_objectParameters.worldPosition);
    const float3 baseReflectivity = lerp(m_dielectricBase, m_pbrParameters.albedo.xyz, m_pbrParameters.metallic);
    
    float finalAO = 1.f;
    
    if (u_rendererData.enableAO != 0 && pbrData.castAO != false)
    {
        finalAO = CalculateAO(pbrData.position.xy);
    }
    
    LightingOutput directLightAccumulation;
    directLightAccumulation.diffuse = 0.f;
    directLightAccumulation.specular = 0.f;
    
    const uint2 tileId = uint2(pbrData.position.xy / 16);
    
    // Direct light
    {
        // Ambiance
        {
            LightingOutput result = CalculateAmbiance(dirToCamera, baseReflectivity);
            directLightAccumulation.diffuse += result.diffuse;
            directLightAccumulation.specular += result.specular;
        }
        
        // Directional Light
        {
            LightingOutput result = CalculateDirectionalLight(u_directionalLight, dirToCamera, baseReflectivity);
            directLightAccumulation.diffuse += result.diffuse;
            directLightAccumulation.specular += result.specular;
        }
        
        // Point Light
        {
            LightingOutput result = CalculatePointLights(dirToCamera, baseReflectivity, tileId);
            directLightAccumulation.diffuse += result.diffuse;
            directLightAccumulation.specular += result.specular;
        }
        
        // Spot Lights
        {
            LightingOutput result = CalculateSpotLights(dirToCamera, baseReflectivity, tileId);
            directLightAccumulation.diffuse += result.diffuse;
            directLightAccumulation.specular += result.specular;
        }
        
        // Sphere Lights
        {
            LightingOutput result = CalculateSphereLights(dirToCamera, baseReflectivity);
            directLightAccumulation.diffuse += result.diffuse;
            directLightAccumulation.specular += result.specular;
        }
        
        // Rectangle Lights
        {
            LightingOutput result = CalculateRectangleLights(dirToCamera, baseReflectivity);
            directLightAccumulation.diffuse += result.diffuse;
            directLightAccumulation.specular += result.specular;
        }
        
        directLightAccumulation.diffuse *= finalAO;
        directLightAccumulation.specular *= finalAO;
        
        directLightAccumulation.diffuse += CalculateInscatteredLight(directLightAccumulation.diffuse);
    }
    
#ifndef SSS
    
    float3 compositeLighting = directLightAccumulation.diffuse + m_pbrParameters.emissive + directLightAccumulation.specular;
    
    //int pointLightCount = GetPointLightCount(uint2(pbrData.position.xy / 16));
    //float value = float(pointLightCount);
    //compositeLighting.rgb = (compositeLighting.rgb * 0.2) + GetGradient(value);
    
    return compositeLighting;
#else
    LightingOutput output;
    output.diffuse = directLightAccumulation.diffuse * finalAO + m_pbrParameters.emissive;
    output.specular = directLightAccumulation.specular * finalAO;
    
    return output;
#endif

}

#endif