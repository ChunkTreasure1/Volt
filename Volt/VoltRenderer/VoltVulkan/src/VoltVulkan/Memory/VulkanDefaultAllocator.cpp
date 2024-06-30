#include "vkpch.h"
#include "VulkanDefaultAllocator.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Memory/VulkanAllocation.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Core/Profiling.h>
#include <VoltRHI/Memory/MemoryUtility.h>
#include <VoltRHI/Utility/HashUtility.h>

#include <vma/VulkanMemoryAllocator.h>

namespace Volt::RHI
{
	VulkanDefaultAllocator::VulkanDefaultAllocator()
	{
		VmaAllocatorCreateInfo info{};
		info.vulkanApiVersion = VK_API_VERSION_1_3;
		info.physicalDevice = GraphicsContext::GetPhysicalDevice()->GetHandle<VkPhysicalDevice>();
		info.device = GraphicsContext::GetDevice()->GetHandle<VkDevice>();
		info.instance = GraphicsContext::Get().GetHandle<VkInstance>();
		info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		if (GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled())
		{
			info.flags |= VMA_ALLOCATOR_CREATE_EXT_DESCRIPTOR_BUFFER_BIT;
		}

		VT_VK_CHECK(vmaCreateAllocator(&info, &m_allocator));
	}

	VulkanDefaultAllocator::~VulkanDefaultAllocator()
	{
		for (int32_t i = static_cast<int32_t>(m_activeImageAllocations.size()) - 1; i >= 0; i--)
		{
			if (!m_activeBufferAllocations.at(i))
			{
				continue;
			}

			VulkanDefaultAllocator::DestroyImageInternal(m_activeImageAllocations.at(i));
		}

		for (int32_t i = static_cast<int32_t>(m_activeBufferAllocations.size()) - 1; i >= 0; i--)
		{
			// #TODO_Ivar: Probably want to take a look at why allocations can be empty here
			if (!m_activeBufferAllocations.at(i))
			{
				continue;
			}

			VulkanDefaultAllocator::DestroyBufferInternal(m_activeBufferAllocations.at(i));
		}

		vmaDestroyAllocator(m_allocator);
	}

	RefPtr<Allocation> VulkanDefaultAllocator::CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromBufferSpec(size, usage, memoryUsage);

		{
			std::scoped_lock lock{ m_bufferAllocationMutex };
			if (auto buffer = m_allocationCache.TryGetBufferAllocationFromHash(hash))
			{
				return buffer;
			}
		}

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 
		bufferInfo.size = size;
		bufferInfo.usage = Utility::GetVkBufferUsageFlags(usage);

		if (GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled())
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}

		VmaMemoryUsage usageFlags = VMA_MEMORY_USAGE_AUTO;
		VmaAllocationCreateFlags createFlags = 0;

		if ((memoryUsage & MemoryUsage::CPU) != MemoryUsage::None)
		{
			usageFlags = VMA_MEMORY_USAGE_CPU_ONLY;
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		else if ((memoryUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		else if ((memoryUsage & MemoryUsage::GPUToCPU) != MemoryUsage::None)
		{
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}

		if ((memoryUsage & MemoryUsage::Dedicated) != MemoryUsage::None)
		{
			createFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = usageFlags;
		allocCreateInfo.flags = createFlags;
		allocCreateInfo.memoryTypeBits = 0;
		allocCreateInfo.preferredFlags = 0;
		allocCreateInfo.priority = 0.f;
		allocCreateInfo.pool = nullptr;
		allocCreateInfo.pUserData = nullptr;

		VmaAllocationInfo allocInfo{};

		RefPtr<VulkanBufferAllocation> allocation = RefPtr<VulkanBufferAllocation>::Create(hash);
		VT_VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &allocCreateInfo, &allocation->m_resource, &allocation->m_allocation, &allocInfo));

		allocation->m_size = size;

		{
			std::scoped_lock lock{ m_bufferAllocationMutex };
			m_activeBufferAllocations.push_back(allocation);
		}
		return allocation;
	}

	RefPtr<Allocation> VulkanDefaultAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		const size_t hash = Utility::GetHashFromImageSpec(imageSpecification, memoryUsage);

		{
			std::scoped_lock lock{ m_imageAllocationMutex };

			if (auto image = m_allocationCache.TryGetImageAllocationFromHash(hash))
			{
				return image;
			}
		}

		const VkImageCreateInfo imageInfo = Utility::GetVkImageCreateInfo(imageSpecification);

		VmaMemoryUsage usageFlags = VMA_MEMORY_USAGE_AUTO;
		VmaAllocationCreateFlags createFlags = 0;

		if ((memoryUsage & MemoryUsage::CPU) != MemoryUsage::None)
		{
			usageFlags = VMA_MEMORY_USAGE_CPU_ONLY;
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		else if ((memoryUsage & MemoryUsage::CPUToGPU) != MemoryUsage::None)
		{
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		else if ((memoryUsage & MemoryUsage::GPUToCPU) != MemoryUsage::None)
		{
			createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}

		if ((memoryUsage & MemoryUsage::Dedicated) != MemoryUsage::None)
		{
			createFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = usageFlags;
		allocCreateInfo.flags = createFlags;
		allocCreateInfo.priority = 1.f;

		VmaAllocationInfo allocInfo{};

		RefPtr<VulkanImageAllocation> allocation = RefPtr<VulkanImageAllocation>::Create(hash);
		VT_VK_CHECK(vmaCreateImage(m_allocator, &imageInfo, &allocCreateInfo, &allocation->m_resource, &allocation->m_allocation, &allocInfo));

		if (!imageSpecification.debugName.empty())
		{
			vmaSetAllocationName(m_allocator, allocation->m_allocation, imageSpecification.debugName.c_str());
		}

		// Get Size
		{
			VmaAllocationInfo info{};
			vmaGetAllocationInfo(m_allocator, allocation->m_allocation, &info);
			allocation->m_size = info.size;
		}

		{
			std::scoped_lock lock{ m_imageAllocationMutex };
			m_activeImageAllocations.push_back(allocation);
		}
		return allocation;
	}

	void VulkanDefaultAllocator::DestroyBuffer(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueBufferAllocationForRemoval(allocation);
	}

	void VulkanDefaultAllocator::DestroyImage(RefPtr<Allocation> allocation)
	{
		m_allocationCache.QueueImageAllocationForRemoval(allocation);
	}

	void VulkanDefaultAllocator::Update()
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

	void VulkanDefaultAllocator::DestroyBufferInternal(RefPtr<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		const VulkanBufferAllocation& bufferAlloc = allocation->AsRef<VulkanBufferAllocation>();
		vmaDestroyBuffer(m_allocator, bufferAlloc.m_resource, bufferAlloc.m_allocation);

		std::scoped_lock lock{ m_bufferAllocationMutex };
		if (const auto it = std::ranges::find(m_activeBufferAllocations, allocation); it != m_activeBufferAllocations.end())
		{
			m_activeBufferAllocations.erase(it);
		}
	}

	void VulkanDefaultAllocator::DestroyImageInternal(RefPtr<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		const VulkanImageAllocation& imageAlloc = allocation->AsRef<VulkanImageAllocation>();
		vmaDestroyImage(m_allocator, imageAlloc.m_resource, imageAlloc.m_allocation);

		std::scoped_lock lock{ m_imageAllocationMutex };
		if (const auto it = std::ranges::find(m_activeImageAllocations, allocation); it != m_activeImageAllocations.end())
		{
			m_activeImageAllocations.erase(it);
		}
	}

	void* VulkanDefaultAllocator::GetHandleImpl() const
	{
		return m_allocator;
	}
}
