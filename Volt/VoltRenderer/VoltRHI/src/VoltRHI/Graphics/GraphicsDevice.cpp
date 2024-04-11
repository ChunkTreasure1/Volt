#include "rhipch.h"
#include "GraphicsDevice.h"

#include "VoltRHI/Buffers/CommandBuffer.h"
#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Graphics/VulkanGraphicsDevice.h>

namespace Volt::RHI
{
	Ref<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			//case GraphicsAPI::D3D12: return CreateRef<D3D12GraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateVulkanGraphicsDevice(deviceInfo);
		}

		return nullptr;
	}

	GraphicsDevice::GraphicsDevice()
	{
	}
}
