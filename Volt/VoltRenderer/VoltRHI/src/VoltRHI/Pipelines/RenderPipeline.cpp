#include "rhipch.h"
#include "RenderPipeline.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Pipelines/VulkanRenderPipeline.h>

namespace Volt::RHI
{
	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanRenderPipeline>(createInfo); break;
		}

		return nullptr;
	}
}