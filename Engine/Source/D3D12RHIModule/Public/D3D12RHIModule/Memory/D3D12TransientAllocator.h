#pragma once

#include <RHIModule/Memory/Allocator.h>
#include <RHIModule/Memory/AllocationCache.h>

namespace Volt::RHI
{
	class TransientHeap;

	class D3D12TransientAllocator : public TransientAllocator
	{
	public:
		D3D12TransientAllocator();
		~D3D12TransientAllocator() override;

		RefPtr<Allocation> CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage) override;
		RefPtr<Allocation> CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage) override;

		void DestroyBuffer(RefPtr<Allocation> allocation) override;
		void DestroyImage(RefPtr<Allocation> allocation) override;

		void Update() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		inline static constexpr uint32_t HEAP_PAGE_SIZE = 128 * 1024 * 1024;

		void CreateDefaultHeaps();

		void DestroyBufferInternal(RefPtr<Allocation> allocation);
		void DestroyImageInternal(RefPtr<Allocation> allocation);

		// There are called if their parent heap has been destroyed for some reason
		void DestroyOrphanBuffer(RefPtr<Allocation> allocation);
		void DestroyOrphanImage(RefPtr<Allocation> allocation);

		VT_NODISCARD RefPtr<TransientHeap> CreateNewImageHeap();
		VT_NODISCARD RefPtr<TransientHeap> CreateNewBufferHeap();

		Vector<RefPtr<TransientHeap>> m_bufferHeaps;
		Vector<RefPtr<TransientHeap>> m_imageHeaps;

		AllocationCache m_allocationCache{};
	};
}
