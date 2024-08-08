#pragma once

#include <cstdint>

namespace Volt::RHI::Utility
{
	template <typename T>
	constexpr T Align(T value, uint64_t alignment)
	{
		return (T)(((uint64_t)value + alignment - 1) & ~(alignment - 1));
	}
}
