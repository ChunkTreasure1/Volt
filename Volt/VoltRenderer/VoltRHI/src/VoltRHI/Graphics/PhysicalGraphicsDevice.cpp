#include "rhipch.h"
#include "PhysicalGraphicsDevice.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h>
#include <VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h>

namespace Volt::RHI
{
	Ref<PhysicalGraphicsDevice> PhysicalGraphicsDevice::Create(const PhysicalDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12PhysicalGraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanPhysicalGraphicsDevice>(deviceInfo); break;
		}

		return nullptr;
	}
}
