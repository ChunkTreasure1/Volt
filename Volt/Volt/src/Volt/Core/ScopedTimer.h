#pragma once

#include "Volt/Log/Log.h"

#include <chrono>
#include <string>

namespace Time
{
	typedef std::chrono::milliseconds::period Milliseconds;
	typedef std::chrono::seconds::period Seconds;
	typedef std::chrono::nanoseconds Nanoseconds;
}

class ScopedTimer
{
public:
	ScopedTimer(const std::string& name)
		: m_name(name)
	{
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	ScopedTimer()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	~ScopedTimer()
	{
		if (!m_name.empty())
		{
			const std::string result = m_name + " finished in " + std::to_string(GetTime()) + "ms!\n";
			VT_CORE_TRACE(result);
		}
	}

	template<typename T = Time::Milliseconds>
	inline float GetTime()
	{
		return std::chrono::duration<float, T>(std::chrono::high_resolution_clock::now() - m_startTime).count();
	}

private:
	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_startTime;
};
