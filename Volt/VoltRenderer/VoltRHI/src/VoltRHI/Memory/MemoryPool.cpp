#include "rhipch.h"
#include "MemoryPool.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Memory/VulkanMemoryPool.h>

namespace Volt::RHI
{
	Ref<MemoryPool> MemoryPool::Create(MemoryUsage memoryUsage)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateScopeRHI<VulkanMemoryPool>(memoryUsage); break;
		}

		return nullptr;
	}
}
