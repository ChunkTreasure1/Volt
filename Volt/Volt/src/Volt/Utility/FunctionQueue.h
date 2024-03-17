#pragma once

#include "Volt/Core/Profiling.h"

#include <vector>
#include <functional>

namespace Volt
{
	class FunctionQueue
	{
	public:
		inline void Push(std::function<void()>&& function)
		{
			m_queue.push_back(std::move(function));
		}

		inline void Flush()
		{
			VT_PROFILE_FUNCTION();
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
}
