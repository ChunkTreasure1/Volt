#pragma once

#include <cstdint>

namespace Volt
{
	typedef uint32_t ResourceHandle;

	namespace Resource
	{
		constexpr ResourceHandle Invalid = std::numeric_limits<ResourceHandle>::max();
	}
}
