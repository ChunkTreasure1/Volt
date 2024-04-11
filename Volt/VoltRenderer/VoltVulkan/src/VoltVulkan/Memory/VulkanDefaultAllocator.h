#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Memory/Allocator.h>
#include <VoltRHI/Memory/AllocationCache.h>

struct VmaAllocator_T;

namespace Volt::RHI
{
	class VulkanDefaultAllocator : public DefaultAllocator
	{
	public:
		VulkanDefaultAllocator();
		~VulkanDefaultAllocator() override;

		Ref<Allocation> CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage) override;
		Ref<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;

		void DestroyBuffer(Ref<Allocation> allocation) override;
		void DestroyImage(Ref<Allocation> allocation) override;

		void Update() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void DestroyBufferInternal(Ref<Allocation> allocation);
		void DestroyImageInternal(Ref<Allocation> allocation);

		VmaAllocator_T* m_allocator = nullptr;

		AllocationCache m_allocationCache{};

		std::vector<Ref<Allocation>> m_activeImageAllocations;
		std::vector<Ref<Allocation>> m_activeBufferAllocations;
	
		std::mutex m_bufferAllocationMutex;
		std::mutex m_imageAllocationMutex;
	};
}
