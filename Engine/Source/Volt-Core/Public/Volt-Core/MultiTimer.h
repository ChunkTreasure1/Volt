#pragma once

#include "Volt-Core/Config.h"

#include <chrono>

namespace Volt
{
	class VTCORE_API MultiTimer
	{
	public:
		MultiTimer(float resetTimeMilli = 500);

		void Accumulate();

		inline const float GetMaxFrameTime() const { return m_currentMaxTime; }
		inline const float GetAverageTime() const { return m_averageTime; }

		const float GetTime() const;

	private:
		float m_accumulation = 0.f;
		uint32_t m_accumulationCount = 0;
		float m_accumulatedMaxTime = -FLT_MAX;

		float m_averageTime = 0.f;
		float m_currentMaxTime = -FLT_MAX;

		const float m_resetTime;

		std::chrono::steady_clock::time_point m_timeAtLastAccumulation;
	};
}
