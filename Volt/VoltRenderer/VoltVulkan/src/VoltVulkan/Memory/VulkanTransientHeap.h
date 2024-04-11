#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Memory/TransientHeap.h>
#include <VoltRHI/Memory/Allocation.h>

struct VkDeviceMemory_T;
struct VkImageCreateInfo;

namespace Volt::RHI
{
	inline static constexpr uint32_t MAX_PAGE_COUNT = 5;

	struct PageAllocation
	{
		VkDeviceMemory_T* handle = nullptr;
		uint64_t size = 0;
		uint64_t alignment = 0;

		uint64_t usedSize = 0;
		uint64_t tail = 0;

		std::vector<AllocationBlock> availableBlocks;

		inline const uint64_t GetRemainingSize() const
		{
			return size - std::min(usedSize, size);
		}

		inline const uint64_t GetRemainingTailSize() const
		{
			return size - tail;
		}
	};

	class VulkanTransientHeap : public TransientHeap
	{
	public:
		VulkanTransientHeap(const TransientHeapCreateInfo& info);
		~VulkanTransientHeap() override;

		Ref<Allocation> CreateBuffer(const TransientBufferCreateInfo& createInfo) override;
		Ref<Allocation> CreateImage(const TransientImageCreateInfo& createInfo) override;
			
		void ForfeitBuffer(Ref<Allocation> allocation) override;
		void ForfeitImage(Ref<Allocation> allocation) override;

		const bool IsAllocationSupported(const uint64_t size, TransientHeapFlags heapFlags) const override;
		const UUID64 GetHeapID() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		std::pair<uint32_t, AllocationBlock> FindNextAvailableBlock(const uint64_t size);
		void ForfeitAllocationBlock(const AllocationBlock& allocBlock);

		const bool IsAllocationSupportedInPage(const PageAllocation& page, const uint64_t size) const;

		void InitializeAsBufferHeap();
		void InitializeAsImageHeap();

		TransientHeapCreateInfo m_createInfo;
		MemoryRequirement m_memoryRequirements;

		std::array<PageAllocation, MAX_PAGE_COUNT> m_pageAllocations;
	
		std::mutex m_allocationMutex;
		UUID64 m_heapId;
	};

	VTVK_API Ref<TransientHeap> CreateVulkanTransientHeap(const TransientHeapCreateInfo& info);
}
