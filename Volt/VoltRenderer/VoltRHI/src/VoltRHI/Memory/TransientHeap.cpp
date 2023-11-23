#include "rhipch.h"
#include "TransientHeap.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Memory/VulkanTransientHeap.h>

namespace Volt::RHI
{
	Ref<TransientHeap> TransientHeap::Create(const TransientHeapCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanTransientHeap>(createInfo); break;
		}

		return nullptr;
	}
}
