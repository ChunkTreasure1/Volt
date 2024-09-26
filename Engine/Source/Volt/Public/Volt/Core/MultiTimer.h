#pragma once

namespace Volt
{
	class MultiTimer
	{
	public:
		MultiTimer(float resetTimeMilli = 500);

		void Accumulate();

		inline const float GetMaxFrameTime() const { return myCurrentMaxTime; }
		inline const float GetAverageTime() const { return myAverageTime; }

		const float GetTime() const;

	private:
		float myAccumulation = 0.f;
		uint32_t myAccumulationCount = 0;
		float myAccumulatedMaxTime = -FLT_MAX;

		float myAverageTime = 0.f;
		float myCurrentMaxTime = -FLT_MAX;

		const float myResetTime;

		std::chrono::steady_clock::time_point myTimeAtLastAccum;
	};
}
