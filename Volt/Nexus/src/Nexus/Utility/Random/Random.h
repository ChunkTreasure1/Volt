#pragma once
#include <random>

namespace Nexus
{
	template<class _T>
	inline _T Random()
	{
		std::random_device device;
		std::mt19937_64 engine(device());
		std::uniform_int_distribution<_T> dist;
		return dist(engine);
	}
}
