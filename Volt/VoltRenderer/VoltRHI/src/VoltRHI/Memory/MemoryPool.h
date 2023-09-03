#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class MemoryPool : public RHIInterface
	{
	public:
		virtual ~MemoryPool() = default;

		static Ref<MemoryPool> Create(MemoryUsage memoryUsage);

	protected:
		MemoryPool() = default;
	};
}
