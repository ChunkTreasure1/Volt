///// Structures /////
struct DirectionalLight
{
	float4 direction;
	float4 colorIntensity;

	float4x4 viewProjection;

	uint castShadows;
	float shadowBias;
	uint2 padding;
};

struct PointLight
{
	float4 position;
	float4 color;

	float intensity;
	float radius;
	float falloff;
	float farPlane;

	float3 padding;
	uint castShadows;

    float4x4 viewProjectionMatrices[6];
};

struct PointLightVisible
{
	int index;
	uint3 padding;
};

struct AnimationData
{
	float4x4 bones[128];
};

// Will be removed
#define MAX_ANIMATION_BONES 128

///// Buffers /////
struct CameraData
{
	float4x4 view;
	float4x4 proj;
	float4x4 viewProj;

	float4x4 inverseView;
	float4x4 inverseProj;
	float4x4 inverseViewProj;

	float4 position;
	float ambianceMultiplier;
	float exposure;

	float nearPlane;
	float farPlane;
};

struct ObjectData
{
	float4x4 transform;
	uint id;
	uint isAnimated;

	float timeSinceCreation;

	uint padding;
};

struct PassData
{
	uint2 targetSize;
	float2 inverseTargetSize;

	float2 inverseFullSize;
	float2 padding;
};

struct SceneData
{
	float timeSinceStart;
	float deltaTime;

	uint pointLightCount;
	float padding;
	
	uint2 tileCount;
	float2 padding2;
};

///// Samplers /////
SamplerState u_linearSampler : register(s0);
SamplerState u_linearPointSampler : register(s1);

SamplerState u_pointSampler : register(s2);
SamplerState u_pointLinearSampler : register(s3);

SamplerState u_linearSamplerClamp : register(s4);
SamplerState u_linearPointSamplerClamp : register(s5);

SamplerState u_pointSamplerClamp : register(s6);
SamplerState u_pointLinearSamplerClamp : register(s7);

SamplerState u_anisotropicSampler : register(s8);
SamplerComparisonState u_shadowSampler : register(s9);