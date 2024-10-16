#include "vtcorepch.h"
#include "Volt-Core/MultiTimer.h"

namespace Volt
{
	MultiTimer::MultiTimer(float resetTimeMilli)
		: m_resetTime(resetTimeMilli)
	{
		m_timeAtLastAccumulation = std::chrono::steady_clock::now();
	}

	void MultiTimer::Accumulate()
	{
		const float timeSinceLastAccum = GetTime();
		m_timeAtLastAccumulation = std::chrono::steady_clock::now();
		m_accumulation += timeSinceLastAccum;
		m_accumulationCount++;
		
		m_accumulatedMaxTime = std::max(timeSinceLastAccum, m_accumulatedMaxTime);

		if (m_accumulation > m_resetTime)
		{
			m_averageTime = m_accumulation / (float)m_accumulationCount;
			m_currentMaxTime = m_accumulatedMaxTime;

			m_accumulationCount = 0;
			m_accumulation = 0.f;
			m_accumulatedMaxTime = -FLT_MAX;
		}
	}

	const float MultiTimer::GetTime() const
	{
		return std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - m_timeAtLastAccumulation).count();
	}
}
