#include "rhipch.h"
#include "Buffer.h"

namespace Volt::RHI
{
	Buffer::Buffer(size_t size)
		: m_size(size)
	{
		m_data = std::make_unique<uint8_t>(size);
	}

	Buffer::Buffer(const void* data, size_t size)
		: m_size(size)
	{
		m_data = std::make_unique<uint8_t>(size);
		memcpy_s(m_data.get(), m_size, data, size);
	}

	Buffer::~Buffer()
	{

	}

	void Buffer::Release()
	{
		m_data = nullptr;
	}

	void Buffer::Resize(size_t size)
	{
		m_data = std::make_unique<uint8_t>(size);
		m_size = size;
	}

	void Buffer::SetData(const void* data, size_t size, size_t offset /* = 0 */)
	{
		memcpy_s(m_data.get() + offset, m_size, data, size);
	}

	void* Buffer::GetData() const
	{
		return m_data.get();
	}

	const bool Buffer::IsValid() const
	{
		return m_data && m_size > 0;
	}

	const size_t Buffer::GetSize() const
	{
		return m_size;
	}
}
