#pragma once

#include "CoreUtilities/Assert.h"
#include "CoreUtilities/Memory.h"

template<typename T, size_t MAX_SIZE>
class StackVector
{
public:
	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator;

	constexpr StackVector() noexcept;
	constexpr StackVector(const StackVector<T, MAX_SIZE>& other) noexcept;
	constexpr StackVector(StackVector<T, MAX_SIZE>&& other) noexcept;
	constexpr StackVector(std::initializer_list<value_type> initializerList) noexcept;

	~StackVector() noexcept;

	constexpr StackVector<T, MAX_SIZE>& operator=(const StackVector<T, MAX_SIZE>& other);
	constexpr StackVector<T, MAX_SIZE>& operator=(std::initializer_list<T> initList);
	constexpr StackVector<T, MAX_SIZE>& operator=(StackVector<T, MAX_SIZE>&& other);

	void Push(const value_type& value);
	void Erase(const size_t index);
	void Clear();

	template<typename... Args>
	constexpr T& EmplaceBack(Args&&... args);

	VT_NODISCARD constexpr const value_type& At(const size_t index) const;
	VT_NODISCARD constexpr value_type& At(const size_t index);
	VT_NODISCARD constexpr size_t Size() const;
	VT_NODISCARD constexpr size_t Capacity() const;
	VT_NODISCARD constexpr bool Empty() const;
	VT_NODISCARD constexpr const value_type* Data() const { return m_ptrBegin; }
	VT_NODISCARD constexpr value_type* Data() { return m_ptrBegin; }

	VT_NODISCARD VT_INLINE const value_type& operator[](const size_t index) const { VT_ASSERT_MSG(index < static_cast<size_t>(m_ptrEnd - m_ptrBegin), "StackVector::operator[] - Out of range!"); return *(m_ptrBegin + index); }
	VT_NODISCARD VT_INLINE value_type& operator[](const size_t index) { VT_ASSERT_MSG(index < static_cast<size_t>(m_ptrEnd - m_ptrBegin), "StackVector::operator[] - Out of range!"); return *(m_ptrBegin + index); }

	VT_INLINE VT_NODISCARD constexpr iterator begin() { return m_ptrBegin; }
	VT_INLINE VT_NODISCARD constexpr iterator end() { return m_ptrEnd; }

	VT_INLINE VT_NODISCARD constexpr const_iterator begin() const { return m_ptrBegin; }
	VT_INLINE VT_NODISCARD constexpr const_iterator end() const { return m_ptrEnd; }

private:
	T* Allocate();

	void InitializeAllocation();

	T* m_ptrBegin = nullptr;
	T* m_ptrEnd = nullptr;
	T* m_ptrCapacity = nullptr;
};

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>::StackVector() noexcept
{
	InitializeAllocation();
}

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>::StackVector(const StackVector<T, MAX_SIZE>& other) noexcept
{
	InitializeAllocation();
	m_ptrEnd = UninitializedCopyPtr(other.m_ptrBegin, other.m_ptrEnd, m_ptrBegin);
}

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>::StackVector(StackVector<T, MAX_SIZE>&& other) noexcept
{
	std::swap(m_ptrBegin, other.m_ptrBegin);
	std::swap(m_ptrEnd, other.m_ptrEnd);
	std::swap(m_ptrCapacity, other.m_ptrCapacity);
}

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>::StackVector(std::initializer_list<T> initializerList) noexcept
{
	InitializeAllocation();

	for (const auto& item : initializerList)
	{
		EmplaceBack(std::move(item));
		if (Size() == MAX_SIZE)
		{
			break;
		}
	}
}

template<typename T, size_t MAX_SIZE>
inline StackVector<T, MAX_SIZE>::~StackVector() noexcept
{
	for (auto it = m_ptrBegin; it != m_ptrEnd; ++it)
	{
		Destruct(it); 
	}

	if (m_ptrBegin)
	{
		Internal::StackFree(m_ptrBegin);
	}
}

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>& StackVector<T, MAX_SIZE>::operator=(const StackVector<T, MAX_SIZE>& other)
{
	if (this != &other)
	{
		if (m_ptrBegin)
		{
			Clear();
			Internal::StackFree(m_ptrBegin);
		}

		InitializeAllocation();
		m_ptrEnd = UninitializedCopyPtr(other.m_ptrBegin, other.m_ptrEnd, m_ptrBegin);
	}

	return *this;
}

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>& StackVector<T, MAX_SIZE>::operator=(std::initializer_list<T> initList)
{
	if (m_ptrBegin)
	{
		Clear();
		Internal::StackFree(m_ptrBegin);
	}

	InitializeAllocation();
	for (const auto& item : initList)
	{
		EmplaceBack(std::move(item));
		if (Size() == MAX_SIZE)
		{
			break;
		}
	}

	return *this;
}

template<typename T, size_t MAX_SIZE>
inline constexpr StackVector<T, MAX_SIZE>& StackVector<T, MAX_SIZE>::operator=(StackVector<T, MAX_SIZE>&& other)
{
	if (this != &other)
	{
		Clear();
		Internal::StackFree(m_ptrBegin);

		std::swap(m_ptrBegin, other.m_ptrBegin);
		std::swap(m_ptrEnd, other.m_ptrEnd);
		std::swap(m_ptrCapacity, other.m_ptrCapacity);
	}

	return *this;
}

template<typename T, size_t MAX_SIZE>
template<typename ...Args>
inline constexpr T& StackVector<T, MAX_SIZE>::EmplaceBack(Args&& ...args)
{
	VT_ASSERT(m_ptrEnd < m_ptrCapacity);

	::new((void*)m_ptrEnd++) value_type(std::forward<Args>(args)...);
	return *(m_ptrEnd - 1);
}

template<typename T, size_t MAX_SIZE>
inline void StackVector<T, MAX_SIZE>::Push(const T& value)
{
	VT_ASSERT(m_ptrEnd < m_ptrCapacity);

	::new((void*)m_ptrEnd++) value_type(value);
}

template<typename T, size_t MAX_SIZE>
inline void StackVector<T, MAX_SIZE>::Erase(const size_t index)
{
	VT_ASSERT(index < static_cast<size_t>(m_ptrEnd - m_ptrBegin));

	if (index == Size() - 1)
	{
		--m_ptrEnd;
		Destruct(m_ptrEnd);
		return;
	}

	for (auto it = m_ptrBegin + 1; it != m_ptrEnd; ++it)
	{
		*(it - 1) = std::move(*it);
	}

	--m_ptrEnd;
	Destruct(m_ptrEnd);
}

template<typename T, size_t MAX_SIZE>
inline void StackVector<T, MAX_SIZE>::Clear()
{
	for (auto it = m_ptrBegin; it != m_ptrEnd; ++it)
	{
		Destruct(it);
	}

	m_ptrEnd = m_ptrBegin;
}

template<typename T, size_t MAX_SIZE>
inline constexpr const T& StackVector<T, MAX_SIZE>::At(const size_t index) const
{
	VT_ASSERT_MSG(index < static_cast<size_t>(m_ptrEnd - m_ptrBegin), "StackVector::At - Out of range!");
	return *(m_ptrBegin + index);
}

template<typename T, size_t MAX_SIZE>
inline constexpr T& StackVector<T, MAX_SIZE>::At(const size_t index)
{
	VT_ASSERT_MSG(index < static_cast<size_t>(m_ptrEnd - m_ptrBegin), "StackVector::At - Out of range!");
	return *(m_ptrBegin + index);
}

template<typename T, size_t MAX_SIZE>
inline constexpr size_t StackVector<T, MAX_SIZE>::Size() const
{
	return (m_ptrEnd - m_ptrBegin);
}

template<typename T, size_t MAX_SIZE>
inline constexpr size_t StackVector<T, MAX_SIZE>::Capacity() const
{
	return (m_ptrCapacity - m_ptrBegin);
}

template<typename T, size_t MAX_SIZE>
inline constexpr bool StackVector<T, MAX_SIZE>::Empty() const
{
	return m_ptrEnd == m_ptrBegin;
}

template<typename T, size_t MAX_SIZE>
inline T* StackVector<T, MAX_SIZE>::Allocate()
{
	return reinterpret_cast<T*>(Internal::StackAllocate(MAX_SIZE * sizeof(T)));
}

template<typename T, size_t MAX_SIZE>
inline void StackVector<T, MAX_SIZE>::InitializeAllocation()
{
	m_ptrBegin = Allocate();
	m_ptrEnd = m_ptrBegin;
	m_ptrCapacity = m_ptrBegin + MAX_SIZE;
}
