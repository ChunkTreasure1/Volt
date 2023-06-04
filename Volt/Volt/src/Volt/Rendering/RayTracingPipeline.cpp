#include "vtpch.h"
#include "RayTracingPipeline.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Core/Graphics/GraphicsDevice.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Rendering/GlobalDescriptorSetManager.h"

namespace Volt
{
	RayTracingPipeline::RayTracingPipeline(const RayTracingPipelineSpecification& specification)
		: mySpecification(specification)
	{
		Invalidate();
	}

	RayTracingPipeline::~RayTracingPipeline()
	{
		Release();
	}

	void RayTracingPipeline::Invalidate()
	{
		Release();

		auto resources = mySpecification.shader->GetResources();
		auto device = GraphicsContext::GetDevice();

		// Pipeline Layout
		{
			// Find global descriptor sets
			for (uint32_t set = 0; set < static_cast<uint32_t>(resources.nullPaddedDescriptorSetLayouts.size()); set++)
			{
				if (GlobalDescriptorSetManager::HasDescriptorSet(set))
				{
					const auto descriptorSetLayout = GlobalDescriptorSetManager::GetDescriptorSet(set)->GetDescriptorSetLayout();
					resources.nullPaddedDescriptorSetLayouts[set] = descriptorSetLayout;
				}
			}

			VkPipelineLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.setLayoutCount = static_cast<uint32_t>(resources.nullPaddedDescriptorSetLayouts.size());
			info.pSetLayouts = resources.nullPaddedDescriptorSetLayouts.data();
			info.pushConstantRangeCount = resources.pushConstantRange.size > 0 ? 1 : 0;
			info.pPushConstantRanges = &resources.pushConstantRange;

			VT_CORE_ASSERT(resources.pushConstantRange.size <= 128, "Push constant range must be less or equal to 128 bytes to support all platforms!");
			VT_VK_CHECK(vkCreatePipelineLayout(device->GetHandle(), &info, nullptr, &myPipelineLayout));
		}

		const auto& stageInfos = mySpecification.shader->GetStageInfos();
		const auto& shaderGroups = mySpecification.shader->GetShaderGroups();

		VkRayTracingPipelineCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		createInfo.stageCount = (uint32_t)stageInfos.size();
		createInfo.pStages = stageInfos.data();
		createInfo.groupCount = (uint32_t)shaderGroups.size();
		createInfo.pGroups = shaderGroups.data();
		createInfo.maxPipelineRayRecursionDepth = mySpecification.maxRecursion;
		createInfo.layout = myPipelineLayout;

		Renderer::GetVulkanFunctions().vkCreateRayTracingPipelinesKHR(device->GetHandle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &createInfo, nullptr, &myPipeline);
	}

	void RayTracingPipeline::Release()
	{
		Renderer::SubmitResourceChange([pipeline = myPipeline, pipelineLayout = myPipelineLayout]()
		{
			if (pipeline != VK_NULL_HANDLE)
			{
				auto device = GraphicsContext::GetDevice();
				vkDestroyPipelineLayout(device->GetHandle(), pipelineLayout, nullptr);
				vkDestroyPipeline(device->GetHandle(), pipeline, nullptr);
			}
		});
	}
}
