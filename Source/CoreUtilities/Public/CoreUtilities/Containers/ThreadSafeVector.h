#pragma once

#include "CoreUtilities/Containers/Vector.h"

#include <mutex>
#include <shared_mutex>

template<typename T>
class ThreadSafeVector : protected Vector<T>
{
public:
	using WriteLock = std::unique_lock<std::shared_mutex>;
	using ReadLock = std::shared_lock<std::shared_mutex>;

	VT_INLINE ThreadSafeVector() noexcept = default;

	VT_INLINE ThreadSafeVector(Vector<T>::size_type count) noexcept
		: Vector<T>(count)
	{}

	VT_INLINE ThreadSafeVector(Vector<T>::size_type count, const T& value) noexcept
		: Vector<T>(count, value)
	{ }

	VT_INLINE ThreadSafeVector(const ThreadSafeVector& other) noexcept
		: Vector<T>(other)
	{};
	
	VT_INLINE ThreadSafeVector(ThreadSafeVector&& other) noexcept
		: Vector<T>(std::move(other))
	{}

	VT_INLINE ~ThreadSafeVector() = default;


	VT_INLINE ThreadSafeVector& operator=(const ThreadSafeVector& other)
	{
		Vector<T>::operator=(other);
		return *this;
	}

	VT_INLINE ThreadSafeVector& operator=(ThreadSafeVector&& other)
	{
		Vector<T>::operator=(std::move(other));
		return *this;
	}

	VT_INLINE constexpr void clear()
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::clear();
	}

	VT_NODISCARD VT_INLINE constexpr bool empty() const
	{
		ReadLock lock{ m_mutex };
		return Vector<T>::empty();
	}

	VT_NODISCARD VT_INLINE constexpr T& back()
	{
		ReadLock lock{ m_mutex };
		return Vector<T>::back();
	}

	VT_NODISCARD VT_INLINE constexpr const T& back() const
	{
		ReadLock lock{ m_mutex };
		return Vector<T>::back();
	}

	VT_INLINE constexpr void pop_back()
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::pop_back();
	}

	VT_INLINE constexpr void push_back(const T& value)
	{
		WriteLock lock{ m_mutex };
		Vector<T>::push_back(value);
	}

	VT_NODISCARD VT_INLINE constexpr T& push_back()
	{ 
		WriteLock lock{ m_mutex };
		return Vector<T>::push_back();
	}
	
	VT_INLINE constexpr void push_back(T&& value)
	{
		WriteLock lock{ m_mutex };
		Vector<T>::push_back(std::move(value));
	}

	template<typename... Args>
	VT_INLINE constexpr T& emplace_back(Args&&... args)
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::emplace_back(std::forward<Args>(args)...);
	}

	VT_NODISCARD VT_INLINE T& operator[](Vector<T>::size_type index)
	{
		ReadLock lock{ m_mutex };
		return Vector<T>::operator[](index);
	}

	VT_NODISCARD VT_INLINE const T& operator[](Vector<T>::size_type index) const
	{
		ReadLock lock{ m_mutex };
		return Vector<T>::operator[](index);
	}

	VT_NODISCARD VT_INLINE Vector<T>::iterator begin()
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::begin();
	}

	VT_NODISCARD VT_INLINE Vector<T>::iterator end()
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::end();
	}

	VT_NODISCARD VT_INLINE Vector<T>::const_iterator begin() const
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::begin();
	}

	VT_NODISCARD VT_INLINE Vector<T>::const_iterator end() const
	{
		WriteLock lock{ m_mutex };
		return Vector<T>::end();
	}

private:
	mutable std::shared_mutex m_mutex;
};
