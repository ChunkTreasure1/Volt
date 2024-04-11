#include "rhipch.h"
#include "Swapchain.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Graphics/VulkanSwapchain.h>
#include <VoltD3D12/Graphics/D3D12Swapchain.h>

namespace Volt::RHI
{
	Ref<Swapchain> Swapchain::Create(GLFWwindow* window)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			//case GraphicsAPI::D3D12: return CreateRef<D3D12Swapchain>(window); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateVulkanSwapchain(window); break;
		}

		return nullptr;
	}
}
