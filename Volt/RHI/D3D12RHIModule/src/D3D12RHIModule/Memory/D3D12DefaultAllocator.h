#pragma once

#include <RHIModule/Memory/Allocator.h>
#include <RHIModule/Memory/AllocationCache.h>

namespace D3D12MA
{
	class Allocator;
}

namespace Volt::RHI
{
	class D3D12DefaultAllocator : public DefaultAllocator
	{
	public:
		D3D12DefaultAllocator();
		~D3D12DefaultAllocator() override;

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

		D3D12MA::Allocator* m_allocator;

		AllocationCache m_allocationCache{};

		Vector<RefPtr<Allocation>> m_activeImageAllocations;
		Vector<RefPtr<Allocation>> m_activeBufferAllocations;

		std::mutex m_bufferAllocationMutex;
		std::mutex m_imageAllocationMutex;
	};
}
