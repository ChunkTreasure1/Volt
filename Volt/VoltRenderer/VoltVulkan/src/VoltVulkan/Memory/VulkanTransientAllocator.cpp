#include "vkpch.h"
#include "VulkanTransientAllocator.h"

#include "VoltVulkan/Memory/VulkanAllocation.h"
#include "VoltVulkan/Common/VulkanHelpers.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Memory/TransientHeap.h>
#include <VoltRHI/Memory/MemoryUtility.h>
#include <VoltRHI/Core/Profiling.h>
#include <VoltRHI/Utility/HashUtility.h>

#include <VoltRHI/RHILog.h>

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

		if (!result)
		{
			RHILog::LogTagged(LogSeverity::Error, "[VulkanTransientAllocation]", "Unable to create buffer of size {0}!", size);
		}

		return result;
	}

	RefPtr<Allocation> VulkanTransientAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromImageSpec(imageSpecification, memoryUsage);
		if (auto image  = m_allocationCache.TryGetImageAllocationFromHash(hash))
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
			if (heap->IsAllocationSupported(memoryRequirement.size, TransientHeapFlags::AllowTextures))
			{
				result = heap->CreateImage(info);
				break;
			}
		}

		if (!result)
		{
			RHILog::LogTagged(LogSeverity::Error, "[VulkanTransientAllocation]", "Unable to create image of size {0}!", memoryRequirement.size);
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
		constexpr uint32_t MAX_IMAGE_HEAP_COUNT = 2;
		constexpr uint32_t MAX_BUFFER_HEAP_COUNT = 1;

		// Buffer heap
		for (uint32_t i = 0; i < MAX_BUFFER_HEAP_COUNT; i++)
		{
			TransientHeapCreateInfo info{};
			info.pageSize = 128 * 1024 * 1024;
			info.flags = TransientHeapFlags::AllowBuffers;
			m_bufferHeaps.push_back(TransientHeap::Create(info));
		}

		// Image heap
		for (uint32_t i = 0; i < MAX_IMAGE_HEAP_COUNT; i++)
		{
			TransientHeapCreateInfo info{};
			info.pageSize = 128 * 1024 * 1024;
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
			RHILog::LogTagged(LogSeverity::Error, "[VulkanTransientAllocator]", "Unable to destroy buffer with heap ID {0}!", static_cast<uint64_t>(allocation->GetHeapID()));
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
			RHILog::LogTagged(LogSeverity::Error, "[VulkanTransientAllocator]", "Unable to destroy image with heap ID {0}!", static_cast<uint64_t>(allocation->GetHeapID()));
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
