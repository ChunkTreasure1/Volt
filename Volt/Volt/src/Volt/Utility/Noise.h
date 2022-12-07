#pragma once

#include <GEM/gem.h>
#include <array>

namespace Volt
{
	class Noise
	{
	public:

		static std::array<gem::vec4, 16> HBAOJitter();

	private:
		Noise() = delete;
	};
}