#pragma once

#include "Volt/Core/Base.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class Shader;

	struct RayTracingPipelineSpecification
	{
		Ref<Shader> shader;
		uint32_t maxRecursion = 1;
	};

	class RayTracingPipeline
	{
	public:
		RayTracingPipeline(const RayTracingPipelineSpecification& specification);
		~RayTracingPipeline();

		void Invalidate();

	private:
		void Release();

		RayTracingPipelineSpecification mySpecification;

		VkPipelineLayout myPipelineLayout = nullptr;
		VkPipeline myPipeline = nullptr;
	};
}
