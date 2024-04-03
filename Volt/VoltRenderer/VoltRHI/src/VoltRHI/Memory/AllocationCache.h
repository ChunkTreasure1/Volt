#pragma once

#include <CoreUtilities/Core.h>

namespace Volt::RHI
{
	class Allocation;
	
	struct AllocationContainer
	{
		Ref<Allocation> allocation;
		size_t framesAlive = 0;
	};
	
	struct AllocationsToRemove
	{
		std::vector<Ref<Allocation>> imageAllocations;
		std::vector<Ref<Allocation>> bufferAllocations;
	};

	class AllocationCache
	{
	public:
		Ref<Allocation> TryGetImageAllocationFromHash(const size_t hash);
		Ref<Allocation> TryGetBufferAllocationFromHash(const size_t hash);
		void QueueImageAllocationForRemoval(Ref<Allocation> alloc);
		void QueueBufferAllocationForRemoval(Ref<Allocation> alloc);
		AllocationsToRemove UpdateAndGetAllocationsToDestroy();

		inline const auto& GetImageAllocations() const { return m_imageAllocations; }
		inline const auto& GetBufferAllocations() const { return m_bufferAllocations; }

	private:
		std::mutex m_imageAllocationMutex;
		std::mutex m_bufferAllocationMutex;

		std::vector<AllocationContainer> m_imageAllocations;
		std::vector<AllocationContainer> m_bufferAllocations;
	};
}
