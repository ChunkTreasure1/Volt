#pragma once

#include <chrono>

namespace Volt::RHI
{
	class ValidationTimer
	{
	public:
		ValidationTimer()
		{
			m_startTime = std::chrono::steady_clock::now();
		}

		inline float GetTimeMilliseconds() const
		{
			return std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - m_startTime).count();
		};

		inline float GetTimeNanoseconds() const
		{
			return std::chrono::duration<float, std::chrono::nanoseconds::period>(std::chrono::steady_clock::now() - m_startTime).count();
		};

		inline float GetTimeMicroseconds() const
		{
			return std::chrono::duration<float, std::chrono::microseconds::period>(std::chrono::steady_clock::now() - m_startTime).count();
		};

		inline float GetTimeSeconds() const
		{
			return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now() - m_startTime).count();
		};


	private:
		std::chrono::steady_clock::time_point m_startTime;
	};
}
