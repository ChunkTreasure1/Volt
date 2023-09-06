#pragma once

#include <VoltRHI/Memory/Allocator.h>

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

	protected:
		void* GetHandleImpl() const override;

	private:
		void CreateDefaultHeaps();

		// There are called if their parent heap has been destroyed for some reason
		void DestroyOrphanBuffer(Ref<Allocation> allocation);
		void DestroyOrphanImage(Ref<Allocation> allocation);

		std::vector<Ref<TransientHeap>> m_bufferHeaps;
		std::vector<Ref<TransientHeap>> m_imageHeaps;
	};
}
