#include "vtpch.h"
#include "MultiTimer.h"

namespace Volt
{
	MultiTimer::MultiTimer(float resetTimeMilli)
		: myResetTime(resetTimeMilli)
	{
		myTimeAtLastAccum = std::chrono::high_resolution_clock::now();
	}

	void MultiTimer::Accumulate()
	{
		const float timeSinceLastAccum = GetTime();
		myTimeAtLastAccum = std::chrono::high_resolution_clock::now();
		myAccumulation += timeSinceLastAccum;
		myAccumulationCount++;
		
		myAccumulatedMaxTime = std::max(timeSinceLastAccum, myAccumulatedMaxTime);

		if (myAccumulation > myResetTime)
		{
			myAverageTime = myAccumulation / (float)myAccumulationCount;
			myCurrentMaxTime = myAccumulatedMaxTime;

			myAccumulationCount = 0;
			myAccumulation = 0.f;
			myAccumulatedMaxTime = -FLT_MAX;
		}
	}

	const float MultiTimer::GetTime() const
	{
		return std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - myTimeAtLastAccum).count();
	}
}
