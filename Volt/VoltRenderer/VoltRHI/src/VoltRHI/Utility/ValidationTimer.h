#pragma once

#include <chrono>

namespace Volt::RHI
{
	class ValidationTimer
	{
	public:
		ValidationTimer()
		{
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		inline float GetTimeMilliseconds() const
		{
			return std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
		};

		inline float GetTimeNanoseconds() const
		{
			return std::chrono::duration<float, std::chrono::nanoseconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
		};

		inline float GetTimeMicroseconds() const
		{
			return std::chrono::duration<float, std::chrono::microseconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
		};

		inline float GetTimeSeconds() const
		{
			return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
		};


	private:
		std::chrono::high_resolution_clock::time_point m_startTime;
	};
}
