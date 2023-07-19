#include "rhipch.h"
#include "GraphicsDevice.h"

#include "VoltRHI/Buffers/CommandBuffer.h"
#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Graphics/MockGraphicsDevice.h>
#include <VoltVulkan/Graphics/VulkanGraphicsDevice.h>

namespace Volt
{
	Ref<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanGraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockGraphicsDevice>(deviceInfo); break;
		}

		return nullptr;
	}

	GraphicsDevice::GraphicsDevice()
	{
	}
}
