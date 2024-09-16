
#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"

#include <string_view>

namespace Volt::RHI
{
	class VTRHI_API RHIResource : public RHIInterface
	{
	public:
		~RHIResource() override = default;
		VT_DELETE_COPY_MOVE(RHIResource);

		virtual constexpr ResourceType GetType() const = 0;
		virtual void SetName(std::string_view name) = 0;
		virtual std::string_view GetName() const = 0;
		virtual const uint64_t GetDeviceAddress() const = 0;
		virtual const uint64_t GetByteSize() const = 0;

	protected:
		RHIResource() = default;
	};
}
