#pragma once

#include "Volt/Log/Log.h"

#include <chrono>

#include <string>
#include <windows.h>

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

	inline float GetTime()
	{
		return std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
	};

private:
	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_startTime;
};