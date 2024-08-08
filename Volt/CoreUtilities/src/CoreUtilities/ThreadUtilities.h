#pragma once

#include "CoreUtilities/Config.h"

#include <thread>

extern VTCOREUTIL_API void SetThreadName(std::thread::native_handle_type dwThreadID, const char* threadName);
