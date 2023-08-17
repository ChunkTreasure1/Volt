#include "rhipch.h"
#include "GraphicsContext.h"

#include <VoltVulkan/Graphics/VulkanGraphicsContext.h>
#include <VoltD3D12/Graphics/D3D12GraphicsContext.h>

namespace Volt::RHI
{
	GraphicsContext::GraphicsContext()
	{
		s_context = this;
	}

	GraphicsContext::~GraphicsContext()
	{
		s_context = nullptr;
	}

	Ref<GraphicsContext> GraphicsContext::Create(const GraphicsContextCreateInfo& createInfo)
	{
		s_graphicsAPI = createInfo.graphicsApi;
		s_logHook = createInfo.loghookInfo;
		s_resourceManagementInfo = createInfo.resourceManagementInfo;

		switch (s_graphicsAPI)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12GraphicsContext>(createInfo); break;
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanGraphicsContext>(createInfo); break;
		}

		return nullptr;
	}

	void GraphicsContext::DestroyResource(std::function<void()>&& function)
	{
		if (s_resourceManagementInfo.resourceDeletionCallback)
		{
			s_resourceManagementInfo.resourceDeletionCallback(std::move(function));
		}
		else
		{
			function();
		}
	}
}
