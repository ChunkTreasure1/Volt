#pragma once

#include <vector>
#include <functional>

class FunctionQueue
{
public:
	inline void Push(std::function<void()>&& function)
	{
		m_queue.push_back(std::move(function));
	}

	inline void Flush()
	{
		for (const auto& f : m_queue)
		{
			if (f)
			{
				f();
			}
		}

		m_queue.clear();
	}

	inline void Clear()
	{
		m_queue.clear();
	}

private:
	std::vector<std::function<void()>> m_queue;
};
