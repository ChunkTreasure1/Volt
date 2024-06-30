#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Memory/Allocation.h"

#include <CoreUtilities/Core.h>
#include <CoreUtilities/Pointers/RefPtr.h>

namespace Volt::RHI
{
	struct AllocationContainer
	{
		RefPtr<Allocation> allocation;
		size_t framesAlive = 0;
	};
	
	struct AllocationsToRemove
	{
		std::vector<RefPtr<Allocation>> imageAllocations;
		std::vector<RefPtr<Allocation>> bufferAllocations;
	};

	class VTRHI_API AllocationCache
	{
	public:
		RefPtr<Allocation> TryGetImageAllocationFromHash(const size_t hash);
		RefPtr<Allocation> TryGetBufferAllocationFromHash(const size_t hash);
		void QueueImageAllocationForRemoval(RefPtr<Allocation> alloc);
		void QueueBufferAllocationForRemoval(RefPtr<Allocation> alloc);
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
