#include "vkpch.h"
#include "VulkanTransientAllocator.h"

#include "VulkanRHIModule/Memory/VulkanAllocation.h"
#include "VulkanRHIModule/Common/VulkanHelpers.h"

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/PhysicalGraphicsDevice.h>
#include <RHIModule/Graphics/GraphicsDevice.h>

#include <RHIModule/Memory/TransientHeap.h>
#include <RHIModule/Memory/MemoryUtility.h>
#include <RHIModule/Core/Profiling.h>
#include <RHIModule/Utility/HashUtility.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanTransientAllocator::VulkanTransientAllocator()
	{
		CreateDefaultHeaps();
	}

	VulkanTransientAllocator::~VulkanTransientAllocator()
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

	RefPtr<Allocation> VulkanTransientAllocator::CreateBuffer(const uint64_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromBufferSpec(size, usage, memoryUsage);
		if (auto buffer = m_allocationCache.TryGetBufferAllocationFromHash(hash))
		{
			return buffer;
		}

		TransientBufferCreateInfo info{};
		info.size = size;
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
			VT_LOGC(LogVerbosity::Error, LogVulkanRHI, "Unable to create buffer of size {0}!", size);
		}

		return result;
	}

	RefPtr<Allocation> VulkanTransientAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromImageSpec(imageSpecification, memoryUsage);
		if (auto image = m_allocationCache.TryGetImageAllocationFromHash(hash))
		{
			return image;
		}

		MemoryRequirement memoryRequirement = Utility::GetImageRequirement(Utility::GetVkImageCreateInfo(imageSpecification));

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
			VT_LOGC(LogVerbosity::Error, LogVulkanRHI, "Unable to create image of size {0}!", memoryRequirement.size);
		}

		return result;
	}

	void VulkanTransientAllocator::DestroyBuffer(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueBufferAllocationForRemoval(allocation);
	}

	void VulkanTransientAllocator::DestroyImage(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueImageAllocationForRemoval(allocation);
	}

	void* VulkanTransientAllocator::GetHandleImpl() const
	{
		return nullptr;
	}

	void VulkanTransientAllocator::CreateDefaultHeaps()
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

	void VulkanTransientAllocator::DestroyBufferInternal(RefPtr<Allocation> allocation)
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
			VT_LOGC(LogVerbosity::Error, LogVulkanRHI, "Unable to destroy buffer with heap ID {0}!", static_cast<uint64_t>(allocation->GetHeapID()));
			DestroyOrphanBuffer(allocation);
		}
	}

	void VulkanTransientAllocator::DestroyImageInternal(RefPtr<Allocation> allocation)
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
			VT_LOGC(LogVerbosity::Error, LogVulkanRHI, "Unable to destroy image with heap ID {0}!", static_cast<uint64_t>(allocation->GetHeapID()));
			DestroyOrphanImage(allocation);
		}
	}

	void VulkanTransientAllocator::DestroyOrphanBuffer(RefPtr<Allocation> allocation)
	{
		auto device = GraphicsContext::GetDevice();
		vkDestroyBuffer(device->GetHandle<VkDevice>(), allocation->GetResourceHandle<VkBuffer>(), nullptr);
	}

	void VulkanTransientAllocator::DestroyOrphanImage(RefPtr<Allocation> allocation)
	{
		auto device = GraphicsContext::GetDevice();
		vkDestroyImage(device->GetHandle<VkDevice>(), allocation->GetResourceHandle<VkImage>(), nullptr);
	}

	VT_NODISCARD RefPtr<TransientHeap> VulkanTransientAllocator::CreateNewImageHeap()
	{
		TransientHeapCreateInfo info{};
		info.pageSize = HEAP_PAGE_SIZE;
		info.flags = TransientHeapFlags::AllowTextures | TransientHeapFlags::AllowRenderTargets;
		
		RefPtr<TransientHeap>& heap = m_imageHeaps.emplace_back();
		heap = TransientHeap::Create(info);

		return heap;
	}

	VT_NODISCARD RefPtr<TransientHeap> VulkanTransientAllocator::CreateNewBufferHeap()
	{
		TransientHeapCreateInfo info{};
		info.pageSize = HEAP_PAGE_SIZE;
		info.flags = TransientHeapFlags::AllowBuffers;

		RefPtr<TransientHeap>& heap = m_bufferHeaps.emplace_back();
		heap = TransientHeap::Create(info);

		return heap;
	}

	void VulkanTransientAllocator::Update()
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
}
