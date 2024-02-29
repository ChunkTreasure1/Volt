#include "vtpch.h"
#include "UUID.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"

#include <random>
#include <unordered_map>

namespace Volt
{
	static std::random_device s_randomDevice;
	static std::mt19937_64 s_Engine(s_randomDevice());
	static std::mt19937 s_engine32(s_randomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;
	static std::uniform_int_distribution<uint32_t> s_uniformDistribution32;

	UUID::UUID()
		: myUUID(s_UniformDistribution(s_Engine))
	{
	}

	void UUID::Serialize(BinaryStreamWriter& streamWriter, const UUID& data)
	{
		streamWriter.Write(data.myUUID);
	}

	UUID32::UUID32()
		: m_uuid(s_uniformDistribution32(s_engine32))
	{
	}
}
