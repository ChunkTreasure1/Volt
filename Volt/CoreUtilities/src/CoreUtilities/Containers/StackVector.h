#pragma once

#include "CoreUtilities/Assert.h"

#include <array>

template<typename T, size_t MAX_SIZE>
class StackVector
{
public:
	StackVector() = default;
	~StackVector() = default;

	StackVector(std::initializer_list<T> initializerList);

	void Push(const T& value);
	void Erase(const size_t index);

	template<typename... Args>
	constexpr T& EmplaceBack(Args&&... args);

	VT_NODISCARD constexpr const T& At(const size_t index) const;
	VT_NODISCARD constexpr T& At(const size_t index);
	VT_NODISCARD constexpr size_t Size() const;
	VT_NODISCARD constexpr bool Empty() const;
	VT_NODISCARD constexpr const T* Data() const { return m_data.data(); }
	VT_NODISCARD constexpr T* Data() { return m_data.data(); }

	VT_NODISCARD VT_INLINE const T& operator[](const size_t index) const { VT_ASSERT(index < m_currentSize);  return m_data[index]; }
	VT_NODISCARD VT_INLINE T& operator[](const size_t index) { VT_ASSERT(index < m_currentSize); return m_data[index]; }

	class Iterator
	{
	public:
		explicit Iterator(T* ptr)
			: m_ptr(ptr)
		{
		}

		explicit Iterator(const T* ptr)
			: m_ptr(const_cast<T*>(ptr))
		{
		}

		VT_INLINE T& operator*() const
		{
			return *m_ptr;
		}

		VT_INLINE Iterator& operator++()
		{
			++m_ptr;
			return *this;
		}

		VT_NODISCARD VT_INLINE const bool operator!=(const Iterator& other) const
		{
			return m_ptr != other.m_ptr;
		}

		VT_NODISCARD VT_INLINE const bool operator==(const Iterator& other) const
		{
			return m_ptr == other.m_ptr;
		}

	private:
		T* m_ptr;
	};

	VT_INLINE Iterator begin() { return m_data.data(); }
	VT_INLINE Iterator end() { return m_data.data() + m_currentSize; }

	VT_INLINE const Iterator begin() const { return Iterator(m_data.data()); }
	VT_INLINE const Iterator end() const { return Iterator(m_data.data() + m_currentSize); }

private:
	std::array<T, MAX_SIZE> m_data;
	size_t m_currentSize = 0;
};

template<typename T, size_t MAX_SIZE>
inline StackVector<T, MAX_SIZE>::StackVector(std::initializer_list<T> initializerList)
{
	for (const auto& item : initializerList)
	{
		m_data[m_currentSize++] = std::move(item);

		if (m_currentSize >= MAX_SIZE)
		{
			break;
		}
	}
}

template<typename T, size_t MAX_SIZE>
template<typename ...Args>
inline constexpr T& StackVector<T, MAX_SIZE>::EmplaceBack(Args&& ...args)
{
	VT_ASSERT(m_currentSize < MAX_SIZE);
	return m_data[m_currentSize++] = T{ std::forward<Args>(args)... };
}

template<typename T, size_t MAX_SIZE>
inline void StackVector<T, MAX_SIZE>::Push(const T& value)
{
	VT_ASSERT(m_currentSize < MAX_SIZE);
	m_data[m_currentSize++] = value;
}

template<typename T, size_t MAX_SIZE>
inline void StackVector<T, MAX_SIZE>::Erase(const size_t index)
{
	VT_ASSERT(m_currentSize > 0);
	VT_ASSERT(index < m_currentSize);

	if (index == m_currentSize - 1)
	{
		m_currentSize--;
		return;
	}

	for (size_t i = index + 1; i < m_currentSize; i++)
	{
		m_data[i - 1] = std::move(m_data[i]);
	}

	m_currentSize--;
}

template<typename T, size_t MAX_SIZE>
inline constexpr const T& StackVector<T, MAX_SIZE>::At(const size_t index) const
{
	VT_ASSERT(m_currentSize > 0);
	VT_ASSERT(index < m_currentSize);

	return m_data.at(index);
}

template<typename T, size_t MAX_SIZE>
inline constexpr T& StackVector<T, MAX_SIZE>::At(const size_t index)
{
	VT_ASSERT(m_currentSize > 0);
	VT_ASSERT(index < m_currentSize);

	return m_data.at(index);
}

template<typename T, size_t MAX_SIZE>
inline constexpr size_t StackVector<T, MAX_SIZE>::Size() const
{
	return m_currentSize;
}

template<typename T, size_t MAX_SIZE>
inline constexpr bool StackVector<T, MAX_SIZE>::Empty() const
{
	return m_currentSize == 0;
}
