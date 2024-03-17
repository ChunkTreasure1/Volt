#include "rhipch.h"
#include "ComputePipeline.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Pipelines/VulkanComputePipeline.h>

namespace Volt::RHI
{
	Ref<ComputePipeline> ComputePipeline::Create(Ref<Shader> shader, bool useGlobalResources)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanComputePipeline>(shader, useGlobalResources);
		}

		return nullptr;
	}
}
