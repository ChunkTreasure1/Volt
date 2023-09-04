#pragma once

#include <VoltRHI/Memory/TransientHeap.h>

namespace Volt::RHI
{
	class VulkanTransientHeap : public TransientHeap
	{
	public:
		VulkanTransientHeap(const TransientHeapCreateInfo& info);
		~VulkanTransientHeap() override;

		Ref<Allocation> CreateBuffer(const TransientBufferCreateInfo& createInfo) override;
		const bool IsAllocationSupported(const uint64_t size, TransientHeapFlags heapFlags) const override;
		
	private:
		struct MemoryRequirement
		{
			uint64_t size;
			uint64_t alignment;
			uint32_t memoryTypeBits;
		};


		void InitializeAsBufferHeap();
		void InitializeAsImageHeap();

		TransientHeapCreateInfo m_createInfo;
		MemoryRequirement m_memoryRequirements;
	};
}
