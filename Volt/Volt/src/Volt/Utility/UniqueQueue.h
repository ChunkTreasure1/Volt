#pragma once

#include <queue>
#include <set>
#include <functional>

namespace Volt
{
	template<typename T>
	concept TriviallyCopyable = std::is_trivially_copyable<T>::value;

	template<TriviallyCopyable T>
	class UniqueQueue
	{
	public:
		inline void Push(const T& value)
		{
			if (m_uniqueSet.find(value) == m_uniqueSet.end())
			{
				m_queue.push(value);
				m_uniqueSet.insert(m_queue.back());
			}
		}

		inline const bool TryPop(T& value)
		{
			if (m_queue.empty())
			{
				return false;
			}

			value = m_queue.front();
			m_queue.pop();

			m_uniqueSet.erase(value);

			return true;
		}

		inline const bool Empty() const { return m_queue.empty(); }
		inline const auto& GetQueue() const { return m_uniqueSet; }

	private:
		std::queue<T> m_queue;
		std::set<T> m_uniqueSet;
	};
}
