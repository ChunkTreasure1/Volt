#pragma once

#include "CoreUtilities/Core.h"

#include <chrono>
#include <filesystem>

namespace TimeUtility
{
	template<typename CLOCK = std::chrono::system_clock>
	VT_NODISCARD VT_INLINE uint64_t GetTimeSinceEpoch()
	{
		auto duration = CLOCK::now().time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		return static_cast<uint64_t>(millis);
	}

	VT_NODISCARD VT_INLINE uint64_t GetLastWriteTime(const std::filesystem::path& filePath)
	{
		const auto lastWriteTime = std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(filePath));
		const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(lastWriteTime.time_since_epoch()).count();

		return static_cast<uint64_t>(millis);
	}
}
