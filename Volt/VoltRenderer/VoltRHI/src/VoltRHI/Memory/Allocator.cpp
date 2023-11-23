#include "rhipch.h"
#include "Allocator.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Memory/VulkanDefaultAllocator.h>
#include <VoltVulkan/Memory/VulkanTransientAllocator.h>

namespace Volt::RHI
{
	Ref<TransientAllocator> TransientAllocator::Create()
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateScope<VulkanTransientAllocator>(); break;
		}

		return nullptr;
	}

	Scope<DefaultAllocator> DefaultAllocator::Create()
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateScope<VulkanDefaultAllocator>(); break;
		}

		return nullptr;
	}
}
