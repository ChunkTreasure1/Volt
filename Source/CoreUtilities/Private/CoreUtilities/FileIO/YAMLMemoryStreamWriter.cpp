#include "cupch.h"
#include "FileIO/YAMLMemoryStreamWriter.h"

Buffer YAMLMemoryStreamWriter::WriteAndGetBuffer() const
{
	Buffer buffer{ m_emitter.size() };
	buffer.Copy(m_emitter.c_str(), m_emitter.size());
	return buffer;
}
