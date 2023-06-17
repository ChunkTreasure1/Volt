#pragma once

#include <GEM/gem.h>

namespace Volt
{
	struct PreethamSkyData
	{
		float turbidity;
		float azimuth;
		float inclination;
		float padding0;
	};

	struct BloomData
	{
		float filterRadius;
		float strength;
		gem::vec2 padding;
	};

	struct HBAOData
	{
		int uvOffsetIndex;
		float negInvR2;
		float multiplier;
		float powExponent;

		gem::vec4 float2Offsets[16];
		gem::vec4 jitters[16];
		gem::vec4 perspectiveInfo;

		gem::vec2 inverseQuarterSize;
		float radiusToScreen;
		float NdotVBias;

		gem::vec2 invResDirection;
		float sharpness;
		float padding;
	};

	struct VoxelSceneData
	{
		gem::vec3 center = { 0.f, 0.f, 0.f };
		float voxelSize = 20.f;

		uint32_t resolution = 256;
		uint32_t padding[3];

		gem::mat4 debugTransform;
	};

	struct AutoExposureData
	{
		gem::vec4ui size;
		gem::vec4 luminanceTime;
	};

	struct SkyboxData
	{
		float textureLod = 0.f;
		float intensity = 1.f;

		gem::vec2 padding;
	};

	struct PreDepthData
	{
		gem::mat4 lastFrameViewProjection = { 1.f };
	};

	struct SceneRendererData
	{
		// Pre Depth
		PreDepthData preDepth{};

		// Preetham
		PreethamSkyData preethamSky{};

		// Skybox 
		SkyboxData skyboxData{};

		// Bloom
		BloomData bloom{};

		// HBAO
		HBAOData hbao{};

		// Voxel GI
		VoxelSceneData voxelGIData{};

		// Auto Exposure
		AutoExposureData autoExposure;
	};
}
