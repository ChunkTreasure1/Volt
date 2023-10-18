#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class PhysicalGraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(PhysicalGraphicsDevice);
		~PhysicalGraphicsDevice() override = default;

		[[nodiscard]] virtual const DeviceVendor GetDeviceVendor() const = 0;
		[[nodiscard]] virtual std::string_view GetDeviceName() const = 0;

		static Ref<PhysicalGraphicsDevice> Create(const PhysicalDeviceCreateInfo& deviceInfo);

	protected:
		PhysicalGraphicsDevice() = default;
	};
}
