#pragma once
#include "VoltRHI/Graphics/PhysicalGraphicsDevice.h"
namespace Volt
{
	class MockPhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		MockPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo);
	};
}
