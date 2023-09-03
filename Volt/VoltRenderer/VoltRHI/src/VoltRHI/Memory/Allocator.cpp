#include "rhipch.h"
#include "Allocator.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Memory/VulkanDefaultAllocator.h>

namespace Volt::RHI
{
	Scope<LinearAllocator> LinearAllocator::Create()
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			//case GraphicsAPI::Vulkan: return CreateScopeRHI<VulkanAllocator>(); break;
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

			case GraphicsAPI::Vulkan: return CreateScopeRHI<VulkanDefaultAllocator>(); break;
		}

		return nullptr;
	}
}
