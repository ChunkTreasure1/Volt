#include "vtpch.h"
#include "BinaryStreamWriter.h"

namespace Volt
{
	void BinaryStreamWriter::WriteToDisk(const std::filesystem::path& targetFilepath)
	{
		std::ofstream stream(targetFilepath, std::ios::out | std::ios::binary);
		stream.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());
		stream.close();
	}

	void BinaryStreamWriter::WriteData(const void* data, const size_t size, const TypeHeader& typeHeader)
	{
		constexpr size_t typeHeaderSize = sizeof(TypeHeader);

		const size_t writeSize = size + typeHeaderSize;
		size_t currentOffset = m_data.size();
		m_data.resize(currentOffset + writeSize);

		memcpy_s(&m_data[currentOffset], writeSize, &typeHeader, typeHeaderSize);
		currentOffset += typeHeaderSize;
	
		memcpy_s(&m_data[currentOffset], writeSize - typeHeaderSize, data, size);
	}

	void BinaryStreamWriter::WriteData(const void* data, const size_t size)
	{
		const size_t writeSize = size;
		size_t currentOffset = m_data.size();
		m_data.resize(currentOffset + writeSize);

		memcpy_s(&m_data[currentOffset], writeSize, data, size);
	}
}
