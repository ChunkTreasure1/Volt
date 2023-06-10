#include "rhipch.h"
#include "PhysicalGraphicsDevice.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Graphics/MockPhysicalGraphicsDevice.h>

namespace Volt
{
	Ref<PhysicalGraphicsDevice> PhysicalGraphicsDevice::Create(const PhysicalDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::Vulkan:
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Mock: return CreateRefRHI<MockPhysicalGraphicsDevice>(deviceInfo); break;
		}

		return nullptr;
	}
}
