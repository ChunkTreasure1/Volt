#include "rhipch.h"
#include "Swapchain.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Graphics/MockSwapchain.h>

namespace Volt
{
	Ref<Swapchain> Swapchain::Create(GLFWwindow* window)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::Vulkan:
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Mock: return CreateRefRHI<MockSwapchain>(window); break;
		}

		return nullptr;
	}
}
