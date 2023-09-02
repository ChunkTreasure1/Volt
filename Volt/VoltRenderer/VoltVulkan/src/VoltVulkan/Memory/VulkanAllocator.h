#pragma once

#include <VoltRHI/Memory/Allocator.h>

struct VmaAllocator_T;

namespace Volt::RHI
{
	class VulkanAllocator : public Allocator
	{
	public:
		VulkanAllocator();
		~VulkanAllocator() override;

		Ref<Allocation> CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage) override;
		Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;

		void DestroyBuffer(Ref<Allocation> allocation) override;
		void DestroyImage(Ref<Allocation> allocation) override;

	protected:
		void* GetHandleImpl() override;

	private:
		VmaAllocator_T* m_allocator = nullptr;

		std::vector<Ref<Allocation>> m_activeImageAllocations;
		std::vector<Ref<Allocation>> m_activeBufferAllocations;
	};
}
