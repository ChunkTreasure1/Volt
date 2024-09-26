#pragma once

#include <chrono>

namespace Time
{
	typedef std::chrono::hours::period Hours;
	typedef std::chrono::minutes::period Minutes;
	typedef std::chrono::seconds::period Seconds;
	typedef std::chrono::milliseconds::period Milliseconds;
	typedef std::chrono::microseconds::period MicroSeconds;
	typedef std::chrono::nanoseconds::period Nanoseconds;
}
