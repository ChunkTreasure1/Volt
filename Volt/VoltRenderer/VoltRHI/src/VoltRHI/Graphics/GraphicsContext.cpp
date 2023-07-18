#include "rhipch.h"
#include "GraphicsContext.h"

#include <VoltMock/Graphics/MockGraphicsContext.h>
#include <VoltVulkan/Graphics/VulkanGraphicsContext.h>
#include <VoltD3D12/Graphics/D3D12GraphicsContext.h>

namespace Volt
{
	Ref<GraphicsContext> GraphicsContext::Create(const GraphicsContextCreateInfo& createInfo)
	{
		s_graphicsAPI = createInfo.graphicsApi;
		switch (s_graphicsAPI)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12GraphicsContext>(createInfo); break;
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanGraphicsContext>(createInfo); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockGraphicsContext>(createInfo); break;
		}

		return nullptr;
	}

	void GraphicsContext::Log(Severity logSeverity, std::string_view message)
	{
		if (!s_logHook.enabled)
		{
			return;
		}

		if (s_logHook.logCallback)
		{
			s_logHook.logCallback(logSeverity, message);
		}
	}
}
