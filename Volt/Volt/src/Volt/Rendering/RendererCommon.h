#pragma once

#include <VoltRHI/Descriptors/ResourceHandle.h>
#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct MeshTaskCommand
	{
		uint32_t drawId;
		uint32_t taskCount;
		uint32_t meshletOffset;
	};

	///// Rendering Structures /////
	struct ViewData
	{
		// Camera
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 inverseView;
		glm::mat4 inverseProjection;
		glm::mat4 viewProjection;
		glm::mat4 inverseViewProjection;
		glm::vec4 cameraPosition;
		glm::vec4 cullingFrustum;
		glm::vec2 depthUnpackConsts;
		float nearPlane;
		float farPlane;
	
		// Render Target
		glm::vec2 renderSize;
		glm::vec2 invRenderSize;

		// Light Culling
		uint32_t tileCountX;

		// Temp lights
		uint32_t pointLightCount;
		uint32_t spotLightCount;
	};

	struct DirectionalLightData
	{
		inline static constexpr uint32_t CASCADE_COUNT = 4;

		glm::vec4 direction;
		glm::vec3 color;
		float intensity;

		uint32_t castShadows = 1;

		float cascadeDistances[CASCADE_COUNT];
		glm::mat4 viewProjections[CASCADE_COUNT];
	};

	struct DirectionalLightInfo
	{
		DirectionalLightData data;
		glm::vec4 projectionBounds[DirectionalLightData::CASCADE_COUNT];
		glm::mat4 views[DirectionalLightData::CASCADE_COUNT];
	};

	struct PointLightData
	{
		glm::vec3 position;
		float radius;

		glm::vec3 color;
		float intensity;

		float falloff;
		glm::vec3 padding;
	};

	struct SpotLightData
	{
		glm::vec3 position;
		float angleAttenuation;

		glm::vec3 color;
		float intensity;

		glm::vec3 direction;
		float range;

		float angle;
		float falloff;
		glm::vec2 padding;
	};
}
