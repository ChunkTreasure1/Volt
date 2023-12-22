#include "vtpch.h"
#include "Algorithms.h"

#include "Volt/Math/Math.h"
#include "Volt/Core/Application.h"
#include "Volt/Core/Threading/ThreadPool.h"
#include "Volt/Core/Base.h"

namespace Volt::Algo
{
	std::vector<std::future<void>> ForEachParallelLockable(std::function<void(uint32_t threadIdx, uint32_t elementIdx)>&& func, uint32_t iterationCount)
	{
		auto& threadPool = Application::GetThreadPool();

		const uint32_t threadCount = std::min(iterationCount, threadPool.GetThreadCount());
		const uint32_t perThreadIterationCount = Math::DivideRoundUp(iterationCount, threadCount);

		std::vector<std::future<void>> futures;
		futures.reserve(threadCount);

		uint32_t iterOffset = 0;
		for (uint32_t i = 0; i < threadCount; i++)
		{
			uint32_t currThreadIterationCount = perThreadIterationCount;
			if (i == threadCount - 1)
			{
				currThreadIterationCount = iterationCount - i * perThreadIterationCount;
			}

			futures.emplace_back(threadPool.SubmitTask([currThreadIterationCount, func, iterOffset, i]() 
			{
				for (uint32_t iter = 0; iter < currThreadIterationCount; iter++)
				{
					func(i, iter + iterOffset);
				}
			}));

			iterOffset += currThreadIterationCount;
		}
		
		return futures;
	}

	void ForEachParallel(std::function<void(uint32_t, uint32_t)>&& func, uint32_t iterationCount)
	{
		VT_CORE_ASSERT(iterationCount > 0, "Iteration count must be greater than zero!");

		auto& threadPool = Application::GetThreadPool();

		const uint32_t threadCount = std::min(iterationCount, threadPool.GetThreadCount());
		const uint32_t perThreadIterationCount = Math::DivideRoundUp(iterationCount, threadCount);

		uint32_t iterOffset = 0;
		for (uint32_t i = 0; i < threadCount; i++)
		{
			uint32_t currThreadIterationCount = perThreadIterationCount;
			if (i == threadCount - 1)
			{
				currThreadIterationCount = iterationCount - i * perThreadIterationCount;
			}

			threadPool.SubmitTask([currThreadIterationCount, func, iterOffset, i]()
			{
				for (uint32_t iter = 0; iter < currThreadIterationCount; iter++)
				{
					func(i, iter + iterOffset);
				}
			});

			iterOffset += currThreadIterationCount;
		}
	}

	uint32_t GetThreadCountFromIterationCount(uint32_t iterationCount)
	{
		VT_CORE_ASSERT(iterationCount > 0, "Iteration count must be greater than zero!");

		auto& threadPool = Application::GetThreadPool();
		const uint32_t threadCount = std::min(iterationCount, threadPool.GetThreadCount());
	
		return threadCount;
	}
}
