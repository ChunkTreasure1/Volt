#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"

namespace Volt::RHI
{
	class VTRHI_API PhysicalGraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COPY_MOVE(PhysicalGraphicsDevice);
		~PhysicalGraphicsDevice() override = default;

		[[nodiscard]] virtual const DeviceVendor GetDeviceVendor() const = 0;
		[[nodiscard]] virtual std::string_view GetDeviceName() const = 0;

		static RefPtr<PhysicalGraphicsDevice> Create(const PhysicalDeviceCreateInfo& deviceInfo);

	protected:
		PhysicalGraphicsDevice() = default;
	};
}
