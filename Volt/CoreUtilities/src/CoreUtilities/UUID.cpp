#include "cupch.h"
#include "UUID.h"

#include <random>
#include <unordered_map>

static std::random_device s_randomDevice;
static std::mt19937_64 s_engine(s_randomDevice());
static std::uniform_int_distribution<uint64_t> s_uniformDistribution64;
static std::uniform_int_distribution<uint32_t> s_uniformDistribution32;

UUID64::UUID64()
	: m_uuid(s_uniformDistribution64(s_engine))
{
}

UUID64::UUID64(uint64_t uuid)
	: m_uuid(uuid)
{
}

UUID32::UUID32()
	: m_uuid(s_uniformDistribution32(s_engine))
{
}

UUID32::UUID32(uint32_t uuid)
	: m_uuid(uuid)
{
}