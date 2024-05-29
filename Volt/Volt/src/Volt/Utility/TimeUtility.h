#pragma once

#include <CoreUtilities/Core.h>

#include <chrono>

namespace Volt::TimeUtility
{
	VT_INLINE uint64_t GetTimeSinceEpoch()
	{
		auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		return static_cast<uint64_t>(millis);
	}
}
