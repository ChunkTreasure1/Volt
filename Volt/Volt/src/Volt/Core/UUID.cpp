#include "vtpch.h"
#include "UUID.h"

#include <random>
#include <unordered_map>

namespace Volt
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::mt19937 s_Engine32(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;
	static std::uniform_int_distribution<uint32_t> s_UniformDistribution32;

	UUID::UUID()
		: myUUID(s_UniformDistribution(s_Engine))
	{
	}

	UUID32::UUID32()
		: myUUID(s_UniformDistribution32(s_Engine32))
	{
	}
}
