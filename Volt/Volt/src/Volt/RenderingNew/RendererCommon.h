#pragma once

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct IndirectGPUCommandNew
	{
		RHI::IndirectCommand command{};
		uint32_t objectId{};
		uint32_t meshId{};
		uint32_t padding[2]{};
	};

	struct IndirectDrawData
	{
		glm::mat4 transform;
		uint32_t meshId{};
		uint32_t vertexStartOffset{};

		uint32_t padding[2];
	};

	///// Rendering Structures /////
	struct CameraDataNew
	{
		glm::mat4 view;
		glm::mat4 projection;

		glm::mat4 inverseView;
		glm::mat4 inverseProjection;

		glm::vec4 position;
	};

	struct DirectionalLightData
	{
		glm::vec4 direction;

		glm::vec3 color;
		float intensity;
	};
}