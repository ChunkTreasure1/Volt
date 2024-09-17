#pragma once

#include <functional>

#include <JobSystem/TaskGraph.h>

namespace Volt::Algo
{
	extern void ForEachParallelLocking(std::function<void(uint32_t threadIdx, uint32_t elementIdx)>&& func, uint32_t iterationCount);
	extern void ForEachParallel(std::function<void(uint32_t, uint32_t)>&& func, uint32_t iterationCount);
	extern [[nodiscard]] uint32_t GetThreadCountFromIterationCount(uint32_t iterationCount);

	template<typename T>
	extern [[nodiscard]] Vector<uint32_t> ElementCountPrefixSum(const Vector<Vector<T>>& elements)
	{
		Vector<uint32_t> prefixSums(elements.size());

		prefixSums.resize(elements.size());
		prefixSums[0] = 0;

		for (size_t i = 1; i < elements.size(); i++)
		{
			size_t sum = 0;

			for (size_t j = 0; j < i; j++)
			{
				sum += elements.at(j).size();
			}

			prefixSums[i] = static_cast<uint32_t>(sum);
		}

		return prefixSums;
	}
}
