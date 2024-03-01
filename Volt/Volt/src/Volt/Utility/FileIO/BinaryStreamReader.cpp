#include "vtpch.h"
#include "BinaryStreamReader.h"

namespace Volt
{
	void BinaryStreamReader::ReadData(void* outData, const TypeHeader& serializedTypeHeader, const TypeHeader& constructedTypeHeader)
	{
		VT_CORE_ASSERT(serializedTypeHeader.baseTypeSize == constructedTypeHeader.baseTypeSize, "Base Type sizes must match!");
		m_stream.read(reinterpret_cast<char*>(outData), serializedTypeHeader.totalTypeSize);
	}

	BinaryStreamReader::BinaryStreamReader(const std::filesystem::path& filePath)
	{
		m_stream = std::ifstream(filePath, std::ios::in | std::ios::binary);
	}

	bool BinaryStreamReader::IsStreamValid() const
	{
		return m_stream.good();
	}

	TypeHeader BinaryStreamReader::ReadTypeHeader()
	{
		constexpr size_t typeHeaderSize = sizeof(TypeHeader);

		TypeHeader result{};
		m_stream.read(reinterpret_cast<char*>(&result), typeHeaderSize);
		return result;
	}
}
