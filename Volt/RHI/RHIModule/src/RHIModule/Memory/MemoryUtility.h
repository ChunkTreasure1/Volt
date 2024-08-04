#pragma once

#include <cstdint>

namespace Volt::RHI::Utility
{
	template <typename T>
	constexpr T Align(T value, uint64_t alignment)
	{
		return (T)(((uint64_t)value + alignment - 1) & ~(alignment - 1));
	}

	inline size_t HashCombine(size_t lhs, size_t rhs)
	{
		return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
	}
}
