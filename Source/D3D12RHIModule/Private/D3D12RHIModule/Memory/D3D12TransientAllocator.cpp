#include "dxpch.h"
#include "D3D12RHIModule/Memory/D3D12TransientAllocator.h"

#include "D3D12RHIModule/Common/D3D12Helpers.h"

#include <RHIModule/Memory/TransientHeap.h>
#include <RHIModule/Utility/HashUtility.h>
#include <RHIModule/Memory/MemoryUtility.h>

#include <d3d12/d3d12.h>

namespace Volt::RHI
{
	D3D12TransientAllocator::D3D12TransientAllocator()
	{
		CreateDefaultHeaps();
	}

	D3D12TransientAllocator::~D3D12TransientAllocator()
	{
		for (const auto& imageAlloc : m_allocationCache.GetImageAllocations())
		{
			DestroyImageInternal(imageAlloc.allocation);
		}

		for (const auto& bufferAlloc : m_allocationCache.GetBufferAllocations())
		{
			DestroyBufferInternal(bufferAlloc.allocation);
		}

		m_bufferHeaps.clear();
		m_imageHeaps.clear();
	}

	RefPtr<Allocation> D3D12TransientAllocator::CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromBufferSpec(size, usage, memoryUsage);
		if (auto buffer = m_allocationCache.TryGetBufferAllocationFromHash(hash))
		{
			return buffer;
		}

		TransientBufferCreateInfo info{};
		info.size = Utility::Align(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
		info.usage = usage;
		info.memoryUsage = memoryUsage;
		info.hash = hash;

		RefPtr<Allocation> result;

		for (const auto& heap : m_bufferHeaps)
		{
			if (heap->IsAllocationSupported(size, TransientHeapFlags::AllowBuffers))
			{
				result = heap->CreateBuffer(info);
				break;
			}
		}

		// If we were not able to allocate in the heap, we will create a new one
		if (!result)
		{
			auto heap = CreateNewBufferHeap();
			if (heap->IsAllocationSupported(size, TransientHeapFlags::AllowBuffers))
			{
				result = heap->CreateBuffer(info);
			}
		}

		if (!result)
		{
			VT_LOGC(Error, LogD3D12RHI, "Unable to create buffer of size {0}!", size);
		}

		return result;
	}
	
	RefPtr<Allocation> D3D12TransientAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromImageSpec(imageSpecification, memoryUsage);
		if (auto image = m_allocationCache.TryGetImageAllocationFromHash(hash))
		{
			return image;
		}

		MemoryRequirement memoryRequirement = Utility::GetMemoryRequirements(Utility::GetD3D12ResourceDesc(imageSpecification));

		TransientImageCreateInfo info{};
		info.imageSpecification = imageSpecification;
		info.size = Utility::Align(memoryRequirement.size, memoryRequirement.alignment);
		info.hash = hash;

		RefPtr<Allocation> result;

		for (const auto& heap : m_imageHeaps)
		{
			if (heap->IsAllocationSupported(info.size, TransientHeapFlags::AllowTextures))
			{
				result = heap->CreateImage(info);
				break;
			}
		}

		// If we were not able to allocate in the heap, we will create a new one
		if (!result)
		{
			auto heap = CreateNewImageHeap();
			if (heap->IsAllocationSupported(info.size, TransientHeapFlags::AllowTextures))
			{
				result = heap->CreateImage(info);
			}
		}

		if (!result)
		{
			VT_LOGC(Error, LogD3D12RHI, "Unable to create image of size {0}!", memoryRequirement.size);
		}

		return result;
	}
	
	void D3D12TransientAllocator::DestroyBuffer(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueBufferAllocationForRemoval(allocation);
	}
	
	void D3D12TransientAllocator::DestroyImage(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueImageAllocationForRemoval(allocation);
	}
	
	void D3D12TransientAllocator::Update()
	{
		const auto allocationsToRemove = m_allocationCache.UpdateAndGetAllocationsToDestroy();

		for (const auto& alloc : allocationsToRemove.bufferAllocations)
		{
			DestroyBufferInternal(alloc);
		}

		for (const auto& alloc : allocationsToRemove.imageAllocations)
		{
			DestroyImageInternal(alloc);
		}
	}
	
	void* D3D12TransientAllocator::GetHandleImpl() const
	{
		return nullptr;
	}
	
	void D3D12TransientAllocator::CreateDefaultHeaps()
	{
		// Buffer heap
		{
			TransientHeapCreateInfo info{};
			info.pageSize = HEAP_PAGE_SIZE;
			info.flags = TransientHeapFlags::AllowBuffers;
			m_bufferHeaps.push_back(TransientHeap::Create(info));
		}

		// Image heap
		{
			TransientHeapCreateInfo info{};
			info.pageSize = HEAP_PAGE_SIZE;
			info.flags = TransientHeapFlags::AllowTextures | TransientHeapFlags::AllowRenderTargets;
			m_imageHeaps.push_back(TransientHeap::Create(info));
		}
	}
	
	void D3D12TransientAllocator::DestroyBufferInternal(RefPtr<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		RefPtr<TransientHeap> parentHeap;

		for (const auto& heap : m_bufferHeaps)
		{
			if (heap->GetHeapID() == allocation->GetHeapID())
			{
				parentHeap = heap;
				break;
			}
		}

		if (parentHeap)
		{
			parentHeap->ForfeitBuffer(allocation);
		}
		else
		{
			VT_LOGC(Error, LogD3D12RHI, "Unable to destroy buffer with heap ID {0}!", static_cast<uint64_t>(allocation->GetHeapID()));
			DestroyOrphanBuffer(allocation);
		}
	}
	
	void D3D12TransientAllocator::DestroyImageInternal(RefPtr<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		RefPtr<TransientHeap> parentHeap;

		for (const auto& heap : m_imageHeaps)
		{
			if (heap->GetHeapID() == allocation->GetHeapID())
			{
				parentHeap = heap;
				break;
			}
		}

		if (parentHeap)
		{
			parentHeap->ForfeitImage(allocation);
		}
		else
		{
			VT_LOGC(Error, LogD3D12RHI, "Unable to destroy image with heap ID {0}!", static_cast<uint64_t>(allocation->GetHeapID()));
			DestroyOrphanImage(allocation);
		}
	}
	
	// #TODO_Ivar: These functions will probably cause issues due to the ComPtrs
	void D3D12TransientAllocator::DestroyOrphanBuffer(RefPtr<Allocation> allocation)
	{
		allocation->GetResourceHandle<ID3D12Resource*>()->Release();
	}
	
	void D3D12TransientAllocator::DestroyOrphanImage(RefPtr<Allocation> allocation)
	{
		allocation->GetResourceHandle<ID3D12Resource*>()->Release();
	}

	RefPtr<TransientHeap> D3D12TransientAllocator::CreateNewImageHeap()
	{
		TransientHeapCreateInfo info{};
		info.pageSize = HEAP_PAGE_SIZE;
		info.flags = TransientHeapFlags::AllowTextures | TransientHeapFlags::AllowRenderTargets;

		RefPtr<TransientHeap>& heap = m_imageHeaps.emplace_back();
		heap = TransientHeap::Create(info);

		return heap;
	}

	RefPtr<TransientHeap> D3D12TransientAllocator::CreateNewBufferHeap()
	{
		TransientHeapCreateInfo info{};
		info.pageSize = HEAP_PAGE_SIZE;
		info.flags = TransientHeapFlags::AllowBuffers;

		RefPtr<TransientHeap>& heap = m_bufferHeaps.emplace_back();
		heap = TransientHeap::Create(info);

		return heap;
	}
}
