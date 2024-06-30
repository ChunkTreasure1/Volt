#pragma once

#include <VoltRHI/Memory/TransientHeap.h>
#include <VoltRHI/Memory/Allocation.h>

struct ID3D12Heap;

namespace Volt::RHI
{
	class D3D12TransientHeap : public TransientHeap
	{
	public:
		D3D12TransientHeap(const TransientHeapCreateInfo& info);
		~D3D12TransientHeap() override;

		RefPtr<Allocation> CreateBuffer(const TransientBufferCreateInfo& createInfo) override;
		RefPtr<Allocation> CreateImage(const TransientImageCreateInfo& createInfo) override;

		void ForfeitBuffer(RefPtr<Allocation> allocation) override;
		void ForfeitImage(RefPtr<Allocation> allocation) override;

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
}
