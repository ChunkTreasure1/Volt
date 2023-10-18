#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include <string_view>

namespace Volt::RHI
{
	class RHIResource : public RHIInterface
	{
	public:
		~RHIResource() override = default;
		VT_DELETE_COMMON_OPERATORS(RHIResource);

		virtual constexpr ResourceType GetType() const = 0;
		virtual void SetName(std::string_view name) = 0;
		virtual const uint64_t GetDeviceAddress() const = 0;

	protected:
		RHIResource() = default;
	};
}
