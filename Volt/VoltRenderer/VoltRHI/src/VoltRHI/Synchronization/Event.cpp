#include "rhipch.h"
#include "Event.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Synchronization/VulkanEvent.h>

namespace Volt::RHI
{
	Ref<Event> Event::Create(const EventCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateVulkanEvent(createInfo);
		}

		return nullptr;
	}
}
