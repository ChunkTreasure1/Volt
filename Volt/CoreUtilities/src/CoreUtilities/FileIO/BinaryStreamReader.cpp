#include "cupch.h"
#include "BinaryStreamReader.h"

#include "zlib.h"



#include <CoreUtilities/Containers/Vector.h>

constexpr size_t COMPRESSION_ENCODING_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(size_t);
constexpr uint32_t COMPRESSED_CHUNK_SIZE = 16384;
constexpr uint32_t MAGIC = 5121;

BinaryStreamReader::BinaryStreamReader(const std::filesystem::path& filePath)
{
	std::ifstream stream(filePath, std::ios::in | std::ios::binary);
	if (stream)
	{
		stream.seekg(0, std::ios::end);
		m_data.resize_uninitialized(stream.tellg());
		stream.seekg(0, std::ios::beg);
		stream.read(reinterpret_cast<char*>(m_data.data()), m_data.size());

		m_streamValid = true;
	}
	else
	{
		return;
	}

	if (m_data.size() < COMPRESSION_ENCODING_HEADER_SIZE)
	{
		m_streamValid = false;
		return;
	}

	// Read compression encoding
	const uint32_t magic = *(uint32_t*)m_data.data();
	if (magic != MAGIC)
	{
		m_streamValid = false;
		return;
	}

	const uint8_t isCompressed = m_data[sizeof(uint32_t)];
	const size_t compressedDataOffset = *reinterpret_cast<size_t*>(&m_data[sizeof(uint32_t) + sizeof(uint8_t)]) + COMPRESSION_ENCODING_HEADER_SIZE;

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
		m_currentOffset += COMPRESSION_ENCODING_HEADER_SIZE;
	}

	m_compressed = isCompressed;
}

BinaryStreamReader::BinaryStreamReader(const std::filesystem::path& filePath, const size_t maxLoadSize)
{
	size_t bytesToLoadCount = maxLoadSize;

	std::ifstream stream(filePath, std::ios::in | std::ios::binary);
	if (stream)
	{
		stream.seekg(0, std::ios::end);
		const size_t size = stream.tellg();
		stream.seekg(0, std::ios::beg);

		bytesToLoadCount = std::min(size, bytesToLoadCount) + COMPRESSION_ENCODING_HEADER_SIZE;
		m_data.resize_uninitialized(bytesToLoadCount);
		stream.read(reinterpret_cast<char*>(m_data.data()), m_data.size());

		m_streamValid = true;
	}
	else
	{
		return;
	}

	if (m_data.size() < COMPRESSION_ENCODING_HEADER_SIZE)
	{
		m_streamValid = false;
		return;
	}

	// Read compression encoding
	const uint32_t magic = *(uint32_t*)m_data.data();
	if (magic != MAGIC)
	{
		m_streamValid = false;
		return;
	}

	const size_t compressedDataOffset = *reinterpret_cast<size_t*>(&m_data[sizeof(uint32_t) + sizeof(uint8_t)]) + COMPRESSION_ENCODING_HEADER_SIZE;
	const bool isCompressed = bool(m_data[sizeof(uint32_t)]) && m_data.size() > compressedDataOffset;

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
		m_currentOffset += COMPRESSION_ENCODING_HEADER_SIZE;
	}

	m_compressed = isCompressed;
}

void BinaryStreamReader::ReadData(void* outData, const TypeHeader& serializedTypeHeader, const TypeHeader& constructedTypeHeader)
{
	VT_UNUSED(constructedTypeHeader);

	memcpy_s(outData, serializedTypeHeader.totalTypeSize, &m_data[m_currentOffset], serializedTypeHeader.totalTypeSize);
	m_currentOffset += serializedTypeHeader.totalTypeSize;
}

bool BinaryStreamReader::Decompress(size_t compressedDataOffset)
{
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

	Vector<uint8_t> tempResult{};

	do
	{
		stream.avail_in = std::min(COMPRESSED_CHUNK_SIZE, static_cast<uint32_t>(m_data.size()) - srcOffset);
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
			tempResult.resize_uninitialized(tempResult.size() + COMPRESSED_CHUNK_SIZE);

			stream.avail_out = COMPRESSED_CHUNK_SIZE;
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

			uint32_t actualOutSize = COMPRESSED_CHUNK_SIZE - stream.avail_out;
			tempResult.resize_uninitialized(dstOffset + actualOutSize);

		}
		while (stream.avail_out == 0);

	}
	while (zLibResult != Z_STREAM_END);

	inflateEnd(&stream);

	if (zLibResult == Z_STREAM_END)
	{
		m_data.erase(m_data.begin(), m_data.begin() + COMPRESSION_ENCODING_HEADER_SIZE);

		m_data.resize_uninitialized(compressedDataOffset + tempResult.size() - COMPRESSION_ENCODING_HEADER_SIZE);
		memcpy_s(&m_data[compressedDataOffset - COMPRESSION_ENCODING_HEADER_SIZE], tempResult.size(), tempResult.data(), tempResult.size());
	}

	return zLibResult == Z_STREAM_END;
}

bool BinaryStreamReader::IsStreamValid() const
{
	return m_streamValid;
}

void BinaryStreamReader::Read(void* data)
{
	TypeHeader typeHeader{};
	TypeHeader serializedTypeHeader = ReadTypeHeader();
	ReadData(data, serializedTypeHeader, typeHeader);
}

void BinaryStreamReader::ResetHead()
{
	m_currentOffset = m_compressed ? 0 : COMPRESSION_ENCODING_HEADER_SIZE;
}

TypeHeader BinaryStreamReader::ReadTypeHeader()
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);

	TypeHeader result = *reinterpret_cast<TypeHeader*>(&m_data[m_currentOffset]);
	m_currentOffset += typeHeaderSize;
	return result;
}

