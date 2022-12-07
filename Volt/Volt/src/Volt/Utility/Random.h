#pragma once

#include <random>

namespace Volt
{
	class Random
	{
	public:
		static int Int(int aMin, int aMax)
		{
			std::uniform_int_distribution<std::mt19937::result_type> tempDist(aMin, aMax);
			return tempDist(myEngine);
		}

		static float Float(float aMin, float aMax)
		{
			std::uniform_real_distribution<float> dist(aMin, aMax);
			return dist(myEngine);
		}
	private:
		Random() = delete;
	
		inline static std::random_device myDevice;
		inline static std::mt19937 myEngine{ myDevice() };

	};
}