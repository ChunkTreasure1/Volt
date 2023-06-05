#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt
{
	struct PhysicalDeviceInfo
	{};

	enum class DeviceVendor
	{
		AMD,
		NVIDIA,
		Intel
	};

	struct DeviceCapabilities
	{
		DeviceVendor deviceVendor;
		std::string_view gpuName;
	};

	class PhysicalGraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(PhysicalGraphicsDevice);
		~PhysicalGraphicsDevice() override = default;

		inline const DeviceCapabilities& GetCapabilities() const { return m_capabilities; }

		static Ref<PhysicalGraphicsDevice> Create(const PhysicalDeviceInfo& deviceInfo);

	protected:
		DeviceCapabilities m_capabilities;

		PhysicalGraphicsDevice() = default;
	};
}
