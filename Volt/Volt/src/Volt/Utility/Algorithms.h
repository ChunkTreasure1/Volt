#pragma once

#include <functional>


namespace Volt::Algo
{
	extern [[nodiscard]] std::vector<std::future<void>> ForEachParallelLockable(std::function<void(uint32_t)>&& func, uint32_t iterationCount);
	extern void ForEachParallel(std::function<void(uint32_t)>&& func, uint32_t iterationCount);
}
