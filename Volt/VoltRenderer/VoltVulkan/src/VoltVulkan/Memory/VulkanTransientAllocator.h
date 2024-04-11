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

		Ref<Allocation> CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage) override;
		Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;

		void DestroyBuffer(Ref<Allocation> allocation) override;
		void DestroyImage(Ref<Allocation> allocation) override;

		void Update() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void CreateDefaultHeaps();

		void DestroyBufferInternal(Ref<Allocation> allocation);
		void DestroyImageInternal(Ref<Allocation> allocation);

		// There are called if their parent heap has been destroyed for some reason
		void DestroyOrphanBuffer(Ref<Allocation> allocation);
		void DestroyOrphanImage(Ref<Allocation> allocation);

		std::vector<Ref<TransientHeap>> m_bufferHeaps;
		std::vector<Ref<TransientHeap>> m_imageHeaps;
	
		AllocationCache m_allocationCache{};
	};
}
