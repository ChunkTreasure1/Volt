#pragma once

#include <list>
#include <shared_mutex>

template<typename T>
class ThreadSafeList : protected std::list<T>
{
public:
	using WriteLock = std::unique_lock<std::shared_mutex>;
	using ReadLock = std::shared_lock<std::shared_mutex>;

	VT_INLINE ThreadSafeList() noexcept = default;
	VT_INLINE ~ThreadSafeList() = default;

	VT_INLINE constexpr void clear()
	{
		WriteLock lock{ m_mutex };
		return std::list<T>::clear();
	}

	template<typename... Args>
	VT_INLINE constexpr T& emplace_back(Args&&... args)
	{
		WriteLock lock{ m_mutex };
		return std::list<T>::emplace_back(std::forward<Args>(args)...);
	}

private:
	mutable std::shared_mutex m_mutex;
};
