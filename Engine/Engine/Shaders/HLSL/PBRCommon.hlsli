struct PBRParamters
{
	float4 albedo;
	float3 normal;
	float metallic;
	float roughness;
};

static PBRParamters m_pbrParameters;

static const float PI = 3.14159265359;
static const float EPSILON = 0.0000001;

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

float3 FresnelSchlick(float HdotV, float3 baseReflectivity)
{
	return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 baseReflectivity, float roughness)
{
	return baseReflectivity + (max(1.f - roughness, baseReflectivity) - baseReflectivity) * pow(max(1.f - cosTheta, 0.f), 5.f);
}

float3 CalculateDirectionalLight(in DirectionalLight light, float3 dirToCamera, float3 baseReflectivity)
{
	const float3 lightDir = normalize(light.direction.xyz);
	const float3 H = normalize(dirToCamera + lightDir);

	// Cook-Torrance BRDF
	const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);
	const float NdotL = max(dot(m_pbrParameters.normal, lightDir), EPSILON);
	const float HdotV = max(dot(H, dirToCamera), 0.0);
	const float NdotH = max(dot(m_pbrParameters.normal, H), 0.0);

	const float distribution = DistributionGGX(NdotH, m_pbrParameters.roughness);
	const float geometric = GeometrySmith(NdotV, NdotL, m_pbrParameters.roughness);
	const float3 fresnel = FresnelSchlick(HdotV, baseReflectivity);

	float3 specular = distribution * geometric * fresnel;
	specular /= 4.0 * NdotV * NdotL;

	float3 kD = 1.0 - fresnel;
	kD *= 1.0 - m_pbrParameters.metallic;

	const float3 result = (kD * m_pbrParameters.albedo.xyz / PI + specular) * NdotL * light.colorIntensity.w * light.colorIntensity.xyz;
	return result;
}

float3 CalculatePointLight(in PointLight light, float3 dirToCamera, float3 baseReflectivity, float3 worldPosition)
{
	const float distance = length(light.position.xyz - worldPosition);

	// Radiance
	const float3 dirToLight = normalize(light.position.xyz - worldPosition);
	const float3 H = normalize(dirToCamera + dirToLight);

	float attenuation = clamp(1.f - distance / light.radius, 0.f, 1.f);
	attenuation *= lerp(attenuation, 1.f, light.falloff);

	const float3 radiance = light.color.xyz * light.intensity * attenuation;

	// Cook-Torrance BRDF
	const float NdotV = max(dot(m_pbrParameters.normal, dirToCamera), EPSILON);
	const float NdotL = max(dot(m_pbrParameters.normal, dirToLight), EPSILON);
	const float HdotV = max(dot(H, dirToCamera), 0.0);
	const float NdotH = max(dot(m_pbrParameters.normal, H), 0.0);

	const float distribution = DistributionGGX(NdotH, m_pbrParameters.roughness);
	const float geometric = GeometrySmith(NdotV, NdotL, m_pbrParameters.roughness);
	const float3 fresnel = FresnelSchlick(HdotV, baseReflectivity);

	float3 specular = distribution * geometric * fresnel;
	specular /= 4.0 * NdotV * NdotL;

	float3 kD = 1.0 - fresnel;
	kD *= 1.0 - m_pbrParameters.metallic;

	const float3 lightStrength = (kD * m_pbrParameters.albedo.xyz / PI + specular) * radiance * NdotL;
	return lightStrength;
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