#include "../Common.hlsli"
#include "../Buffers.hlsli"

Texture2D u_BRDFLut : register(t11);
TextureCube u_irradianceTexture : register(t12);
TextureCube u_radianceTexture : register(t13);

Texture2D u_directionalShadowMap : register(t14);
TextureCube u_pointLightShadowMap : register(t15);

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
};

static ShaderParameters m_shaderParameters;
static PBRParamters m_pbrParameters;
static ObjectParameters m_objectParameters;

static const float3 m_dielectricBase = 0.04;
static const float PI = 3.14159265359;
static const float EPSILON = 0.0000001;

static const uint NUM_STEPS_INT = 100;
static const float NUM_STEPS = float(NUM_STEPS_INT);
static const float SCATTERING = 0.7f;

static const float4x4 DITHER_PATTERN = float4x4(
	0.f, 0.5f, 0.125f, 0.625f,
	0.75f, 0.22f, 0.875f, 0.375f,
	0.1875f, 0.6875f, 0.0625f, 0.5625f,
	0.9375f, 0.4375f, 0.8125f, 0.3125f);

float DistributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = NdotH * (a2 - 1.f) + 1.f;
	denom = PI * denom * denom;

	return a2 / max(denom, EPSILON);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = roughness + 1.f;
	float k = (r * r) / 8.f;
	float ggx1 = NdotV / (NdotV * (1.f - k) + k);
	float ggx2 = NdotL / (NdotL * (1.f - k) + k);

	return ggx1 * ggx2;
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

float3 FresnelSchlick(float3 baseReflectivity, float HdotV)
{
	return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

float3 FresnelSchlickRoughness(float3 baseReflectivity, float cosTheta, float roughness)
{
	return baseReflectivity + (max(1.f - roughness, baseReflectivity) - baseReflectivity) * pow(1.f - cosTheta, 5.f);
}

// Returns number of mipmap levels for specular IBL environment map.
uint QueryRadianceTextureLevels()
{
	uint width, height, levels;
	u_radianceTexture.GetDimensions(0, width, height, levels);
	return levels;
}

float InterleavedGradientNoise(float2 pos)
{
	const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
	return frac(magic.z * dot(pos, magic.xy));
}

float2 VogelDiskSample(uint sampleIndex, uint sampleCount, float phi)
{
	const float goldenAngle = 2.4f;

	float r = sqrt(sampleIndex + 0.5f) / sqrt(sampleCount);
	float theta = sampleIndex * goldenAngle * phi;

	return r * float2(cos(theta), sin(theta));
}

float CalculateScattering(float cosTheta)
{
	return (1.f - SCATTERING * SCATTERING) / (4.f * PI * pow(1.f + SCATTERING * SCATTERING - 2.f * SCATTERING * cosTheta, 1.5f));
}

float CalculateDirectionalShadow(in DirectionalLight light)
{
	float result = 1.f;

	const float4 lightViewCoords = mul(light.viewProjection, float4(m_objectParameters.worldPosition, 1.f));

	const float2 projectedCoords = float2(lightViewCoords.x / lightViewCoords.w * 0.5f + 0.5f, -lightViewCoords.y / lightViewCoords.w * 0.5f + 0.5f);
	if (saturate(projectedCoords.x) == projectedCoords.x &&
		saturate(projectedCoords.y) == projectedCoords.y)
	{
		result = 0.f;

		float2 shadowSize;
		u_directionalShadowMap.GetDimensions(shadowSize.x, shadowSize.y);

		float2 texelSize = 1.f / shadowSize;

		const float noise = InterleavedGradientNoise(m_shaderParameters.position.xy);
		for (uint i = 0; i < 16; i++)
		{
			float2 sampleOffset = VogelDiskSample(i, 16, noise);
			result += u_directionalShadowMap.SampleCmpLevelZero(u_shadowSampler, projectedCoords.xy + sampleOffset * texelSize * 3.f, lightViewCoords.z / lightViewCoords.w - 0.0001f).r * (1.f / 16.f);
		}
	}

	return result;
}

float CalculatePointLightShadow(in PointLight light)
{
	const float3 lightToPoint = normalize(m_objectParameters.worldPosition - light.position.xyz);
	const float dist = distance(m_objectParameters.worldPosition.xyz, light.position.xyz);

	float3 sampleOffsetDirections[20] =
	{
		float3( 1,  1,  1), float3( 1, -1,  1), float3(-1, -1,  1), float3(-1,  1,  1), 
		float3( 1,  1, -1), float3( 1, -1, -1), float3(-1, -1, -1), float3(-1,  1, -1),
		float3( 1,  1,  0), float3( 1, -1,  0), float3(-1, -1,  0), float3(-1,  1,  0),
		float3( 1,  0,  1), float3(-1,  0,  1), float3( 1,  0, -1), float3(-1,  0, -1),
		float3( 0,  1,  1), float3( 0, -1,  1), float3( 0, -1, -1), float3( 0,  1, -1)
	};   

	float result = 0.f;
	const float bias = 0.15f;
	const int samples = 20;
	const float diskRadius = 0.05f;

	for (int i = 0; i < samples; i++)
	{
		const float closestDepth = u_pointLightShadowMap.SampleLevel(u_pointSamplerClamp, normalize(lightToPoint + sampleOffsetDirections[i] * diskRadius), 0).r * light.farPlane;
		if (dist - bias > closestDepth)
		{
			result += 1.f;
		}
	}

	return result / float(samples);
}

float3 CalculateDirectionalLight(in DirectionalLight light, float3 dirToCamera, float3 baseReflectivity)
{
	const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);

	const float3 Li = normalize(light.direction.xyz);
	const float3 Lradiance = light.colorIntensity.xyz * light.colorIntensity.w;
	const float3 Lh = normalize(Li + dirToCamera);

	const float cosLi = max(0.f, dot(m_pbrParameters.normal, Li));
	const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));

	const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
	const float D = DistributionGGX(cosLh * cosLh, m_pbrParameters.roughness);
	const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);

	const float3 kd = (1.f - F) * (1.f - m_pbrParameters.metallic);
	const float3 diffuseBRDF = kd * m_pbrParameters.albedo.xyz;

	float3 specularBRDF = (F * D * G) / max(EPSILON, 4.f * cosLi * NdotV);
	specularBRDF = clamp(specularBRDF, 0.f, 10.f);

	float shadow = 1.f;
	if (light.castShadows == 1)
	{
		shadow = CalculateDirectionalShadow(light);
	}

	return (diffuseBRDF + specularBRDF) * Lradiance * cosLi * shadow;

	///*const float3 rayVector = m_objectParameters.worldPosition - u_cameraData.position.xyz;
	//
	//const float rayLength = length(rayVector);
	//const float stepLength = rayLength / NUM_STEPS;
	//const float3 rayDirection = rayVector / rayLength;
	//
	//float3 step = rayVector * stepLength;
	//
	//float3 position = u_cameraData.position.xyz;
	//float3 volumetricComponent = 0.f;
	//
	//for (uint i = 0; i < NUM_STEPS_INT; ++i)
	//{
	//	const float4 lightViewCoords = mul(light.viewProjection, float4(m_objectParameters.worldPosition, 1.f));
	//
	//	float3 projectedCoords = lightViewCoords.xyz / lightViewCoords.w;
	//	projectedCoords.x = projectedCoords.x * 0.5f + 0.5f;
	//	projectedCoords.y = -projectedCoords.y * 0.5f + 0.5f;
	//
	//	if (saturate(projectedCoords.x) == projectedCoords.x &&
	//		saturate(projectedCoords.y) == projectedCoords.y)
	//	{
	//		const float shadowDepth = u_directionalShadowMap.SampleLevel(u_pointSampler, projectedCoords.xy, 0).r;
	//
	//		if (shadowDepth > projectedCoords.z)
	//		{
	//			volumetricComponent += CalculateScattering(dot(rayDirection, -lightDir)) * light.colorIntensity.xyz;
	//		}
	//	}
	//	position += step;
	//}
	//
	//volumetricComponent /= NUM_STEPS;*/
}

float3 CalculatePointLight(in PointLight light, float3 dirToCamera, float3 baseReflectivity, float3 worldPosition)
{
	const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);

	const float3 Li = normalize(light.position.xyz - worldPosition);
	const float lightDistance = length(light.position.xyz - worldPosition);
	const float3 Lh = normalize(Li + dirToCamera);

	float attenuation = clamp(1.f - (lightDistance * lightDistance) / (light.radius * light.radius), 0.f, 1.f);
	attenuation *= lerp(attenuation, 1.f, light.falloff);

	const float3 Lradiance = light.color.xyz * light.intensity * attenuation;

	const float cosLi = max(0.f, dot(m_pbrParameters.normal, Li));
	const float cosLh = max(0.f, dot(m_pbrParameters.normal, Lh));

	const float3 F = FresnelSchlickRoughness(baseReflectivity, max(0.f, dot(Lh, dirToCamera)), m_pbrParameters.roughness);
	const float D = DistributionGGX(cosLh * cosLh, m_pbrParameters.roughness);
	const float G = GaSchlickGGX(cosLi, NdotV, m_pbrParameters.roughness);

	const float3 kd = (1.f - F) * (1.f - m_pbrParameters.metallic);
	const float3 diffuseBRDF = kd * m_pbrParameters.albedo.xyz;

	float3 specularBRDF = (F * D * G) / max(EPSILON, 4.f * cosLi * NdotV);
	specularBRDF = clamp(specularBRDF, 0.f, 10.f);

	float shadow = 1.f;
	if (light.castShadows == 1)
	{
		//shadow = 1.f - CalculatePointLightShadow(light);
	}

	return (diffuseBRDF + specularBRDF) * Lradiance * cosLi * shadow;
}

float3 CalculatePointLights(float3 dirToCamera, float3 baseReflectivity, float3 worldPosition)
{
	float3 lightAccumulation = 0.f;

	for (uint i = 0; i < u_sceneData.pointLightCount; i++)
	{
		lightAccumulation += CalculatePointLight(u_pointLights[i], dirToCamera, baseReflectivity, worldPosition);
	}

	return lightAccumulation;
}

float3 CalculateAmbiance(float3 dirToCamera, float3 baseReflectivity)
{
	const float NdotV = max(0.f, dot(m_pbrParameters.normal, dirToCamera));
	const float3 irradiance = u_irradianceTexture.Sample(u_linearSampler, m_pbrParameters.normal).rgb;

	const float3 F = FresnelSchlick(baseReflectivity, NdotV);
	const float3 kD = lerp(1.f - F, 0.f, m_pbrParameters.metallic);

	const float3 diffuseIBL = kD * m_pbrParameters.albedo.xyz * irradiance;

	uint radianceTextureLevels = QueryRadianceTextureLevels();

	const float3 R = 2.f * NdotV * m_pbrParameters.normal - dirToCamera;
	const float3 specularIrradiance = u_radianceTexture.SampleLevel(u_linearSampler, R, m_pbrParameters.roughness * radianceTextureLevels).rgb;

	float2 brdf = u_BRDFLut.Sample(u_linearSamplerClamp, float2(NdotV, m_pbrParameters.roughness)).rg;
	float3 specularIBL = (baseReflectivity * brdf.x + brdf.y) * specularIrradiance;

	return (diffuseIBL + specularIBL);
}

float3 CalculatePBR(float4 albedo, float4 material, float4 normalEmissive, float4 worldPosition, float4 projectionPosition)
{
	m_pbrParameters.albedo = albedo;
	m_pbrParameters.normal = normalize(normalEmissive.rgb);
	m_pbrParameters.metallic = material.r;
	m_pbrParameters.roughness = material.g;
	m_pbrParameters.emissive = float3(material.ba, normalEmissive.a);

	m_objectParameters.worldPosition = worldPosition.xyz;
	m_shaderParameters.position = projectionPosition;

	const float3 dirToCamera = normalize(u_cameraData.position.xyz - worldPosition.xyz);
	const float3 baseReflectivity = lerp(m_dielectricBase, m_pbrParameters.albedo.xyz, m_pbrParameters.metallic);

	float3 lightAccumulation = 0.f;
	lightAccumulation += CalculateDirectionalLight(u_directionalLight, dirToCamera, baseReflectivity);
	lightAccumulation += CalculatePointLights(dirToCamera, baseReflectivity, worldPosition.xyz);
	lightAccumulation += CalculateAmbiance(dirToCamera, baseReflectivity) * u_cameraData.ambianceMultiplier;
	lightAccumulation += m_pbrParameters.emissive;
	
	return lightAccumulation;
}