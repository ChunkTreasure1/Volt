#pragma once

#include <gem/gem.h>

namespace Volt
{
	///// Structures /////
	struct PointLight
	{
		gem::vec4 position;
		gem::vec4 color;

		float intensity;
		float radius;
		float falloff;
		float farPlane;

		gem::vec3 padding;
		uint32_t castShadows = 0;
		gem::mat4 viewProjectionMatrices[6];
	};

	struct DirectionalLight
	{
		gem::vec4 direction = { 0.f, 0.f, 0.f, 0.f };
		gem::vec4 colorIntensity = { 0.f, 0.f, 0.f, 0.f };

		gem::mat4 viewProjection;

		uint32_t castShadows;
		float shadowBias = 0.00001f;
		uint32_t padding[2];
	};

	///// Buffers /////
	struct InstanceData
	{
		uint32_t objectBufferId;
	};

	struct CameraData
	{
		gem::mat4 view;
		gem::mat4 proj;
		gem::mat4 viewProj;

		gem::mat4 inverseView;
		gem::mat4 inverseProj;
		gem::mat4 inverseViewProj;

		gem::vec4 position;

		float ambianceMultiplier = 0.5f;
		float exposure = 1.f;

		float nearPlane;
		float farPlane;
	};

	struct ObjectData
	{
		gem::mat4 transform;
		uint32_t id;
		uint32_t isAnimated;

		float timeSinceCreation = 0.f;

		uint32_t padding[1];
	};

	struct PassData
	{
		gem::vec2ui	targetSize;
		gem::vec2 inverseTargetSize;

		gem::vec2 inverseFullSize;
		gem::vec2 padding;
	};

	struct AnimationData
	{
		gem::mat4 bones[128];
	};

	struct SceneData
	{
		float timeSinceStart = 0.f;
		float deltaTime = 0.f;

		uint32_t pointLightCount = 0;

		float padding = 0.f;
	};

	struct GTAOConstants
	{
		gem::vec2ui viewportSize;
		gem::vec2 viewportPixelSize; // .zw == 1.0 / ViewportSize.xy

		gem::vec2 depthUnpackConsts;
		gem::vec2 cameraTanHalfFov;

		gem::vec2 NDCToViewMul;
		gem::vec2 NDCToViewAdd;

		gem::vec2 NDCToViewMul_x_PixelSize;
		float effectRadius; // world (viewspace) maximum size of the shadow
		float effectFalloffRange;

		float radiusMultiplier;
		float finalValuePower;

		float sampleDistributionPower;
		float thinOccluderCompensation;
		float depthMIPSamplingOffset;
		int noiseIndex = 0; // frameIndex % 64 if using TAA or 0 otherwise

		gem::vec2 hzbUVFactor;
	};

	struct HZBData
	{
		gem::vec2 dispatchThreadIdToBufferUV;
		gem::vec2 inverseSize;
		gem::vec2 inputViewportMaxBounds;
		uint32_t firstLod;
		uint32_t isFirstPass;
	};
}