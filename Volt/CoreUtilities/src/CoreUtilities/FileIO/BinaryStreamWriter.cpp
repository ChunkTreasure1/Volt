#include "cupch.h"
#include "BinaryStreamWriter.h"

#include "zlib.h"

void BinaryStreamWriter::WriteToDisk(const std::filesystem::path& targetFilepath, bool compress, size_t compressedDataOffset)
{
	constexpr size_t compressionEncodingHeaderSize = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(size_t);
	constexpr uint32_t MAGIC = 5121;

	std::ofstream stream(targetFilepath, std::ios::out | std::ios::binary);

	const uint8_t* writePtr = m_data.data();
	size_t size = m_data.size();

	Vector<uint8_t> compressedData;
	if (compress)
	{
		compressedData.resize(compressionEncodingHeaderSize);

		*(uint32_t*)(compressedData.data()) = MAGIC; // First we set a magic value
		compressedData[sizeof(uint32_t)] = 1; // First byte tells if file is compressed

		// Next 8 bytes tells the offset to where the compressed data starts
		memcpy_s(&compressedData[sizeof(uint32_t) + sizeof(uint8_t)], sizeof(size_t), &compressedDataOffset, sizeof(size_t));

		compressedDataOffset += compressionEncodingHeaderSize;

		compressedData.resize(compressedDataOffset);
		if (GetCompressed(compressedData, compressedDataOffset))
		{
			memcpy_s(&compressedData[compressionEncodingHeaderSize], compressedDataOffset - compressionEncodingHeaderSize, m_data.data(), compressedDataOffset - compressionEncodingHeaderSize);

			writePtr = compressedData.data();
			size = compressedData.size();
		}
	}

	if (writePtr == m_data.data())
	{
		// Insert 0 at beginning to flag that file is uncompressed
		// We also insert the magic value
		std::array<uint8_t, compressionEncodingHeaderSize> emptyEncodingHeader;
		emptyEncodingHeader.fill(0);

		*(uint32_t*)(emptyEncodingHeader.data()) = MAGIC;

		m_data.insert(m_data.begin(), emptyEncodingHeader.begin(), emptyEncodingHeader.end());
		writePtr = m_data.data();
		size = m_data.size();
	}

	stream.write(reinterpret_cast<const char*>(writePtr), size);
	stream.close();
}

bool BinaryStreamWriter::GetCompressed(Vector<uint8_t>& result, size_t compressedDataOffset)
{
	constexpr uint32_t CHUNK_SIZE = 16384;
	constexpr size_t compressionEncodingHeaderSize = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(size_t);

	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	int32_t zLibResult = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	if (zLibResult != Z_OK)
	{
		return false;
	}

	int32_t shouldFlush;
	uint32_t srcOffset = static_cast<uint32_t>(compressedDataOffset - compressionEncodingHeaderSize);

	do
	{
		stream.avail_in = std::min(CHUNK_SIZE, static_cast<uint32_t>(m_data.size()) - srcOffset);
		uint8_t* bytePtr = &m_data[srcOffset];
		srcOffset += stream.avail_in;

		shouldFlush = srcOffset == m_data.size() ? Z_FINISH : Z_NO_FLUSH;
		stream.next_in = bytePtr;

		do
		{
			size_t dstOffset = result.size();
			result.resize(result.size() + CHUNK_SIZE);

			stream.avail_out = CHUNK_SIZE;
			stream.next_out = &result[dstOffset];

			zLibResult = deflate(&stream, shouldFlush);
			assert(zLibResult != Z_STREAM_ERROR);

			uint32_t actualOutSize = CHUNK_SIZE - stream.avail_out;
			result.resize(dstOffset + actualOutSize);

		}
		while (stream.avail_out == 0);
		assert(stream.avail_in == 0);

	}
	while (shouldFlush != Z_FINISH);
	assert(zLibResult == Z_STREAM_END);

	deflateEnd(&stream);
	return true;
}

void BinaryStreamWriter::WriteTypeHeader(const TypeHeader& typeHeader)
{
	constexpr size_t typeHeaderSize = sizeof(TypeHeader);

	const size_t offset = m_data.size();
	m_data.resize(m_data.size() + typeHeaderSize);
	memcpy_s(&m_data[offset], typeHeaderSize, &typeHeader, typeHeaderSize);
}

void BinaryStreamWriter::WriteData(const void* data, const size_t size)
{
	assert(size > 0 && "Write size must be greater than zero!");

	const size_t writeSize = size;
	const size_t currentOffset = m_data.size();
	m_data.resize(currentOffset + writeSize);

	memcpy_s(&m_data[currentOffset], writeSize, data, size);
}

size_t BinaryStreamWriter::Write(const void* data, const size_t size)
{
	TypeHeader header{};
	header.totalTypeSize = static_cast<uint32_t>(size);

	WriteTypeHeader(header);
	WriteData(data, size);

	return m_data.size();
}
