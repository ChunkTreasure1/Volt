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

		inline const DeviceCapabilities& GetCapabilities() const { return m_capabilities; }

		static Ref<PhysicalGraphicsDevice> Create(const PhysicalDeviceCreateInfo& deviceInfo);

	protected:
		DeviceCapabilities m_capabilities;
		PhysicalGraphicsDevice() = default;
	};
}
