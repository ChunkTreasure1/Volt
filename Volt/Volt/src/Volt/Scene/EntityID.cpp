#include "vtpch.h"
#include "EntityID.h"

#include <random>

namespace Volt
{
	static std::random_device s_randomDevice;
	static std::mt19937 s_engine32(s_randomDevice());
	static std::uniform_int_distribution<uint32_t> s_uniformDistribution32;

	EntityID::EntityID()
		: m_uuid(s_uniformDistribution32(s_engine32))
	{
	}
}
