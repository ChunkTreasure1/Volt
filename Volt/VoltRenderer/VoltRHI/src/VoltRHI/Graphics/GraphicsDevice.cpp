#include "rhipch.h"
#include "GraphicsDevice.h"

#include "VoltRHI/Buffers/CommandBuffer.h"
#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Graphics/VulkanGraphicsDevice.h>
#include <VoltD3D12/Graphics/D3D12GraphicsDevice.h>

namespace Volt::RHI
{
	Ref<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12GraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanGraphicsDevice>(deviceInfo); break;
		}

		return nullptr;
	}

	GraphicsDevice::GraphicsDevice()
	{
	}
}
