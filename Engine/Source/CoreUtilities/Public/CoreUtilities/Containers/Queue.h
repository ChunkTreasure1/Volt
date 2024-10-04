#pragma once

#include "CoreUtilities/Containers/Vector.h"

template<typename T>
class Queue
{
public:
	Queue() = default;
	Queue(size_t capacity)
		: m_buffer(capacity), m_capacity(capacity)
	{}

	VT_INLINE void Enqueue(const T& value)
	{
		if (m_size == m_capacity)
		{
			Grow(m_size + 1);
		}

		m_buffer[m_tail] = value;
		m_tail = (m_tail + 1) % m_capacity;
		++m_size;
	}

	VT_INLINE bool Dequeue(T& result)
	{
		if (m_size == 0)
		{
			return false;
		}

		result = m_buffer[m_head];
		m_head = (m_head + 1) % m_capacity;
		--m_size;
		return true;
	}

	VT_INLINE bool Empty() const
	{
		return m_size == 0;
	}

	VT_INLINE size_t Size() const
	{
		return m_size;
	}

	VT_INLINE void Clear()
	{
		m_buffer.clear();
		m_head = 0;
		m_tail = 0;
		m_size = 0;
	}

private:
	VT_INLINE void Grow(size_t newSize)
	{
		const size_t newCapacity = std::max(newSize, m_buffer.size() * 2);
		m_buffer.resize(newCapacity);
		m_capacity = newCapacity;
	}

	Vector<T> m_buffer;
	size_t m_head = 0;
	size_t m_tail = 0;
	size_t m_size = 0;
	size_t m_capacity = 0;
};
