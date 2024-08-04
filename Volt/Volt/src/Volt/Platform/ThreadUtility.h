#pragma once

#include <thread>

extern void SetThreadName(std::thread::native_handle_type dwThreadID, const char* threadName);
