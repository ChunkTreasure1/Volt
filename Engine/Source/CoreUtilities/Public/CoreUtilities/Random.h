#pragma once

#include "CoreUtilities/CompilerTraits.h"

#include <random>

class Random
{
public:
	VT_INLINE static int Int(int aMin, int aMax)
	{
		std::uniform_int_distribution<std::mt19937::result_type> tempDist(aMin, aMax);
		return tempDist(s_engine);
	}

	VT_INLINE static float Float(float aMin, float aMax)
	{
		std::uniform_real_distribution<float> dist(aMin, aMax);
		return dist(s_engine);
	}
private:
	Random() = delete;

	inline thread_local static std::random_device s_device;
	inline thread_local static std::mt19937 s_engine{ s_device() };
};
