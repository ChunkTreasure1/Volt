#include "vtpch.h"
#include "Volt/Utility/Algorithms.h"

#include "Volt/Math/Math.h"
#include "Volt/Core/Application.h"
#include "Volt/Core/Base.h"

#include <JobSystem/JobSystem.h>

namespace Volt::Algo
{
	void ForEachParallelLocking(std::function<void(uint32_t threadIdx, uint32_t elementIdx)>&& func, uint32_t iterationCount)
	{
		const uint32_t threadCount = std::min(iterationCount, std::thread::hardware_concurrency());
		const uint32_t perThreadIterationCount = iterationCount / threadCount;

		TaskGraph taskGraph{};

		uint32_t iterOffset = 0;
		for (uint32_t i = 0; i < threadCount; i++)
		{
			uint32_t currThreadIterationCount = perThreadIterationCount;
			if (i == threadCount - 1)
			{
				currThreadIterationCount = iterationCount - i * perThreadIterationCount;
			}

			taskGraph.AddTask([currThreadIterationCount, func, iterOffset, i]()
			{
				for (uint32_t iter = 0; iter < currThreadIterationCount; iter++)
				{
					func(i, iter + iterOffset);
				}
			});

			iterOffset += currThreadIterationCount;
		}
	
		taskGraph.ExecuteAndWait();
	}

	void ForEachParallel(std::function<void(uint32_t, uint32_t)>&& func, uint32_t iterationCount)
	{
		VT_ASSERT_MSG(iterationCount > 0, "Iteration count must be greater than zero!");

		const uint32_t threadCount = std::min(iterationCount, std::thread::hardware_concurrency());
		const uint32_t perThreadIterationCount = iterationCount / threadCount;

		uint32_t iterOffset = 0;
		for (uint32_t i = 0; i < threadCount; i++)
		{
			uint32_t currThreadIterationCount = perThreadIterationCount;
			if (i == threadCount - 1)
			{
				currThreadIterationCount = iterationCount - i * perThreadIterationCount;
			}

			JobSystem::CreateAndRunJob([currThreadIterationCount, func, iterOffset, i]()
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
		VT_ASSERT_MSG(iterationCount > 0, "Iteration count must be greater than zero!");

		const uint32_t threadCount = std::min(iterationCount, std::thread::hardware_concurrency());
	
		return threadCount;
	}
}
