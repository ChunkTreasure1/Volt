#pragma once

#include <VoltRHI/Memory/Allocator.h>

namespace Volt::RHI
{
	class VulkanLinearAllocator : public LinearAllocator
	{
	public:
		VulkanLinearAllocator();
		~VulkanLinearAllocator() override;

		Ref<Allocation> CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage) override;

		Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;
		Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, Ref<MemoryPool> pool, MemoryUsage memoryUsage) override;

		void DestroyBuffer(Ref<Allocation> allocation) override;
		void DestroyImage(Ref<Allocation> allocation) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		uint64_t m_pageSize = 0;
		uint64_t m_memoryBlockMinSize = 0;

	};
}
