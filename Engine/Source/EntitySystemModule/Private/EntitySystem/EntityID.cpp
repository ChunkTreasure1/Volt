#include "espch.h"
#include "EntityID.h"

#include <random>

#include <CoreUtilities/FileIO/BinaryStreamReader.h>
#include <CoreUtilities/FileIO/BinaryStreamWriter.h>

namespace Volt
{
	static std::random_device s_randomDevice;
	static std::mt19937 s_engine32(s_randomDevice());
	static std::uniform_int_distribution<uint32_t> s_uniformDistribution32;

	EntityID::EntityID()
		: m_uuid(s_uniformDistribution32(s_engine32))
	{
	}

	void EntityID::Serialize(BinaryStreamWriter& streamWriter, const EntityID& data)
	{
		streamWriter.Write(data.m_uuid);
	}

	void EntityID::Deserialize(BinaryStreamReader& streamReader, EntityID& outData)
	{
		streamReader.Read(outData.m_uuid);
	}

	EntityID EntityID::Null()
	{
		return EntityID(0);
	}
}
