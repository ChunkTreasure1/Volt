#pragma once

#include <queue>
#include <mutex>
#include <shared_mutex>

template<typename T>
class ThreadSafeQueue : protected std::queue<T>
{
public:
	using WriteLock = std::unique_lock<std::shared_mutex>;
	using ReadLock = std::shared_lock<std::shared_mutex>;

	inline ~ThreadSafeQueue() = default;
	inline ThreadSafeQueue()
	{
		clear();
	}

	inline ThreadSafeQueue(const ThreadSafeQueue&) = delete;
	inline ThreadSafeQueue(ThreadSafeQueue&&) = delete;

	inline ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
	inline ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

	inline const bool try_pop(T& holder)
	{
		WriteLock lock{ m_mutex };
		if (std::queue<T>::empty())
		{
			return false;
		}

		holder = std::move(std::queue<T>::front());
		std::queue<T>::pop();

		return true;
	}

	inline void clear()
	{
		WriteLock lock{ m_mutex };
		while (!std::queue<T>::empty())
		{
			std::queue<T>::pop();
		}
	}

	inline const bool empty() const
	{
		ReadLock lock{ m_mutex };
		return std::queue<T>::empty();
	}

	inline const size_t size() const
	{
		ReadLock lock{ m_mutex };
		return std::queue<T>::size();
	}

	inline void push(const T& obj)
	{
		WriteLock lock{ m_mutex };
		std::queue<T>::push(obj);
	}

	template<typename... Args>
	inline void emplace(Args&&... args)
	{
		WriteLock lock{ m_mutex };
		std::queue<T>::emplace(std::forward<Args>(args)...);
	}

private:
	mutable std::shared_mutex m_mutex;
};
