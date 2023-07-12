#include "rhipch.h"
#include "PhysicalGraphicsDevice.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Graphics/MockPhysicalGraphicsDevice.h>
#include <VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h>

namespace Volt
{
	Ref<PhysicalGraphicsDevice> PhysicalGraphicsDevice::Create(const PhysicalDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanPhysicalGraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockPhysicalGraphicsDevice>(deviceInfo); break;
		}

		return nullptr;
	}
}
