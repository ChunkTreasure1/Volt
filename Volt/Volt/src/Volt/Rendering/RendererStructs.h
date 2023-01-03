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

	struct VisibleLightIndex
	{
		int32_t index;
		gem::vec3ui padding;
	};
	
	struct LightCullingData
	{
		inline static constexpr uint32_t TILE_SIZE = 16;

		gem::vec2ui	tileCount = { 0 };
		gem::vec2 padding;
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

		gem::vec2ui tileCount = { 0 };
		gem::vec2 padding2;
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