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
			myQueue.push_back(std::move(function));
		}

		inline void Flush()
		{
			VT_PROFILE_FUNCTION();
			for (const auto& f : myQueue)
			{
				f();
			}

			myQueue.clear();
		}

	private:
		std::vector<std::function<void()>> myQueue;
	};
}