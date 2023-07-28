#pragma once
#include "VoltRHI/Graphics/PhysicalGraphicsDevice.h"

namespace Volt::RHI
{
	class MockPhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		MockPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo);
		~MockPhysicalGraphicsDevice() override = default;

	protected:
		void* GetHandleImpl() override;
	};
}
