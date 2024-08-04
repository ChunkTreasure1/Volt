#pragma once

#include "VulkanRHIModule/Core.h"

#include <RHIModule/Memory/Allocator.h>
#include <RHIModule/Memory/AllocationCache.h>

struct VmaAllocator_T;

namespace Volt::RHI
{
	class VulkanDefaultAllocator : public DefaultAllocator
	{
	public:
		VulkanDefaultAllocator();
		~VulkanDefaultAllocator() override;

		RefPtr<Allocation> CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage) override;
		RefPtr<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;

		void DestroyBuffer(RefPtr<Allocation> allocation) override;
		void DestroyImage(RefPtr<Allocation> allocation) override;

		void Update() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void DestroyBufferInternal(RefPtr<Allocation> allocation);
		void DestroyImageInternal(RefPtr<Allocation> allocation);

		VmaAllocator_T* m_allocator = nullptr;

		AllocationCache m_allocationCache{};

		Vector<RefPtr<Allocation>> m_activeImageAllocations;
		Vector<RefPtr<Allocation>> m_activeBufferAllocations;
	
		std::mutex m_bufferAllocationMutex;
		std::mutex m_imageAllocationMutex;
	};
}
