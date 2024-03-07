#include "vtpch.h"
#include "BinaryStreamReader.h"

#include "zlib.h"

namespace Volt
{
	BinaryStreamReader::BinaryStreamReader(const std::filesystem::path& filePath)
	{
		constexpr size_t compressionEncodingHeaderSize = sizeof(uint8_t) + sizeof(size_t);

		std::ifstream stream(filePath, std::ios::in | std::ios::binary);
		if (stream)
		{
			stream.seekg(0, std::ios::end);
			m_data.resize(stream.tellg());
			stream.seekg(0, std::ios::beg);
			stream.read(reinterpret_cast<char*>(m_data.data()), m_data.size());

			m_streamValid = true;
		}
		else
		{
			return;
		}

		// Read compression encoding
		const uint8_t isCompressed = m_data[0];
		const size_t compressedDataOffset = *reinterpret_cast<size_t*>(&m_data[1]) + compressionEncodingHeaderSize;

		if (isCompressed)
		{
			if (!Decompress(compressedDataOffset))
			{
				m_streamValid = false; 
			}
		}
		else
		{
			// Decompress automatically removes compression encoding header
			m_currentOffset += compressionEncodingHeaderSize;
		}
	}

	void BinaryStreamReader::ReadData(void* outData, const TypeHeader& serializedTypeHeader, const TypeHeader& constructedTypeHeader)
	{
		VT_CORE_ASSERT(serializedTypeHeader.baseTypeSize == constructedTypeHeader.baseTypeSize, "Base Type sizes must match!");
		memcpy_s(outData, serializedTypeHeader.totalTypeSize, &m_data[m_currentOffset], serializedTypeHeader.totalTypeSize);
		m_currentOffset += serializedTypeHeader.totalTypeSize;
	}

	bool BinaryStreamReader::Decompress(size_t compressedDataOffset)
	{
		constexpr uint32_t CHUNK_SIZE = 16384;
		constexpr size_t compressionEncodingHeaderSize = sizeof(uint8_t) + sizeof(size_t);

		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = 0;
		stream.next_in = Z_NULL;

		int32_t zLibResult = inflateInit(&stream);
		if (zLibResult != Z_OK)
		{
			return false;
		}

		uint32_t srcOffset = static_cast<uint32_t>(compressedDataOffset);

		std::vector<uint8_t> tempResult{};

		do 
		{
			stream.avail_in = std::min(CHUNK_SIZE, static_cast<uint32_t>(m_data.size()) - srcOffset);
			if (stream.avail_in == 0)
			{
				break;
			}

			uint8_t* bytePtr = &m_data[srcOffset];
			srcOffset += stream.avail_in;
			stream.next_in = bytePtr;

			do 
			{
				size_t dstOffset = tempResult.size();
				tempResult.resize(tempResult.size() + CHUNK_SIZE);

				stream.avail_out = CHUNK_SIZE;
				stream.next_out = &tempResult[dstOffset];

				zLibResult = inflate(&stream, Z_NO_FLUSH);
				assert(zLibResult != Z_STREAM_ERROR);

				switch (zLibResult)
				{
					case Z_NEED_DICT:
						zLibResult = Z_DATA_ERROR;
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
					{
						inflateEnd(&stream);
						return false;
					}
				}

				uint32_t actualOutSize = CHUNK_SIZE - stream.avail_out;
				tempResult.resize(dstOffset + actualOutSize);

			} while (stream.avail_out == 0);

		} while (zLibResult != Z_STREAM_END);

		inflateEnd(&stream);

		if (zLibResult == Z_STREAM_END)
		{
			m_data.erase(m_data.begin(), m_data.begin() + compressionEncodingHeaderSize);

			m_data.resize(compressedDataOffset + tempResult.size() - compressionEncodingHeaderSize);
			memcpy_s(&m_data[compressedDataOffset - compressionEncodingHeaderSize], tempResult.size(), tempResult.data(), tempResult.size());
		}

		return zLibResult == Z_STREAM_END;
	}

	bool BinaryStreamReader::IsStreamValid() const
	{
		return m_streamValid;
	}

	TypeHeader BinaryStreamReader::ReadTypeHeader()
	{
		constexpr size_t typeHeaderSize = sizeof(TypeHeader);

		TypeHeader result = *reinterpret_cast<TypeHeader*>(&m_data[m_currentOffset]);
		m_currentOffset += typeHeaderSize;
		return result;
	}
}
