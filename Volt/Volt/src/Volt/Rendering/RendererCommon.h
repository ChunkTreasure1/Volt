#pragma once

#include "Volt/Rendering/Resources/ResourceHandle.h"

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct IndirectGPUCommandNew
	{
		RHI::IndirectDrawCommand command{};
		uint32_t objectId{};
		uint32_t meshId{};
		uint32_t meshletId{};
		uint32_t padding{};
	};

	struct IndirectDrawData
	{
		glm::mat4 transform;
		uint32_t meshId{};
		uint32_t vertexStartOffset{};
		uint32_t materialId{};

		uint32_t padding;
	};

	struct GPUMeshNew
	{
		ResourceHandle vertexPositions;
		ResourceHandle vertexMaterial;
		ResourceHandle vertexAnimation;
		ResourceHandle indexBuffer;
	};

	struct IndirectMeshTaskCommand
	{
		RHI::IndirectMeshTasksCommand command{};
		uint32_t objectId{};
		uint32_t meshId{};
		uint32_t padding[3];
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
		inline static constexpr uint32_t CASCADE_COUNT = 5;

		glm::vec4 direction;
		glm::vec3 color;
		float intensity;

		uint32_t castShadows = 1;

		float cascadeDistances[CASCADE_COUNT];
		glm::mat4 viewProjections[CASCADE_COUNT];
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
