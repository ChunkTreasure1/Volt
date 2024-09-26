#pragma once

#include "CoreUtilities/Config.h"

#include <thread>

enum class ThreadPriority : uint8_t
{
	High,
	Medium,
	Low
};

namespace Thread
{
	extern VTCOREUTIL_API void SetThreadName(std::thread::native_handle_type threadHandle, std::string_view threadName);
	extern VTCOREUTIL_API void SetThreadPriority(std::thread::native_handle_type threadHandle, ThreadPriority priority);
	extern VTCOREUTIL_API void AssignThreadToCore(std::thread::native_handle_type threadHandle, uint64_t affinityMask);
	extern VTCOREUTIL_API std::thread::native_handle_type GetCurrentThreadHandle();
}
