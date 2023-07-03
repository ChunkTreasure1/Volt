#pragma once

#include <glm/glm.hpp>

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
		glm::vec2 padding;
	};

	struct HBAOData
	{
		int uvOffsetIndex;
		float negInvR2;
		float multiplier;
		float powExponent;

		glm::vec4 float2Offsets[16];
		glm::vec4 jitters[16];
		glm::vec4 perspectiveInfo;

		glm::vec2 inverseQuarterSize;
		float radiusToScreen;
		float NdotVBias;

		glm::vec2 invResDirection;
		float sharpness;
		float padding;
	};

	struct VoxelSceneData
	{
		glm::vec3 center = { 0.f, 0.f, 0.f };
		float voxelSize = 20.f;

		uint32_t resolution = 256;
		uint32_t padding[3];

		glm::mat4 debugTransform;
	};

	struct AutoExposureData
	{
		glm::uvec4 size;
		glm::vec4 luminanceTime;
	};

	struct SkyboxData
	{
		float textureLod = 0.f;
		float intensity = 1.f;

		glm::vec2 padding;
	};

	struct PreDepthData
	{
		glm::mat4 lastFrameViewProjection = { 1.f };
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
