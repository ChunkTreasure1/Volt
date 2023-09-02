#include "rhipch.h"
#include "Allocator.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Memory/VulkanAllocator.h>

namespace Volt::RHI
{
	Scope<Allocator> Allocator::Create()
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateScopeRHI<VulkanAllocator>(); break;
		}

		return nullptr;
	}
}
