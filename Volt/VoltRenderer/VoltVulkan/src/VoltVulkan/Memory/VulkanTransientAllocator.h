#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Memory/Allocator.h>
#include <VoltRHI/Memory/AllocationCache.h>

namespace Volt::RHI
{
	class TransientHeap;

	class VulkanTransientAllocator : public TransientAllocator
	{
	public:
		VulkanTransientAllocator();
		~VulkanTransientAllocator() override;

		RefPtr<Allocation> CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage) override;
		RefPtr<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;

		void DestroyBuffer(RefPtr<Allocation> allocation) override;
		void DestroyImage(RefPtr<Allocation> allocation) override;

		void Update() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void CreateDefaultHeaps();

		void DestroyBufferInternal(RefPtr<Allocation> allocation);
		void DestroyImageInternal(RefPtr<Allocation> allocation);

		// There are called if their parent heap has been destroyed for some reason
		void DestroyOrphanBuffer(RefPtr<Allocation> allocation);
		void DestroyOrphanImage(RefPtr<Allocation> allocation);

		std::vector<RefPtr<TransientHeap>> m_bufferHeaps;
		std::vector<RefPtr<TransientHeap>> m_imageHeaps;
	
		AllocationCache m_allocationCache{};
	};
}
