#include "rhipch.h"
#include "DeviceQueue.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Graphics/MockDeviceQueue.h>
#include <VoltVulkan/Graphics/VulkanDeviceQueue.h>
#include <VoltD3D12/Graphics/D3D12DeviceQueue.h>

namespace Volt::RHI
{
	Ref<DeviceQueue> DeviceQueue::Create(const DeviceQueueCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12DeviceQueue>(createInfo); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanDeviceQueue>(createInfo); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockDeviceQueue>(createInfo); break;
		}

		return nullptr;
	}
}
