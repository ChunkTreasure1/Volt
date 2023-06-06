#include "rhipch.h"
#include "GraphicsContext.h"

#include <VoltMock/Graphics/MockGraphicsContext.h>

namespace Volt
{
	Ref<GraphicsContext> GraphicsContext::Create(const GraphicsContextCreateInfo& createInfo)
	{
		s_graphicsAPI = createInfo.graphicsApi;
		switch (s_graphicsAPI)
		{
			case GraphicsAPI::Vulkan:
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockGraphicsContext>(createInfo); break;
		}

		return nullptr;
	}
}
