#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class RHIResource : public RHIInterface
	{
	public:
		~RHIResource() override = default;
		VT_DELETE_COMMON_OPERATORS(RHIResource);

		virtual constexpr ResourceType GetType() const = 0;

	protected:
		RHIResource() = default;
	};
}
