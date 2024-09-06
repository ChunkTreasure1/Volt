#pragma once

#include "CoreUtilities/Config.h"

#include "CoreUtilities/Time/Time.h"

class VTCOREUTIL_API ScopedTimer
{
public:
	ScopedTimer();
	~ScopedTimer() = default;

	template<typename T = Time::Milliseconds>
	inline float GetTime()
	{
		return std::chrono::duration<float, T>(std::chrono::steady_clock::now() - m_startTime).count();
	}

private:
	std::chrono::steady_clock::time_point m_startTime;
};
