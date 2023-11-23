#include "rhipch.h"
#include "SamplerState.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Images/VulkanSamplerState.h>

namespace Volt::RHI
{
	Ref<SamplerState> SamplerState::Create(const SamplerStateCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateRef<VulkanSamplerState>(createInfo); break;
		}

		return nullptr;
	}
}
