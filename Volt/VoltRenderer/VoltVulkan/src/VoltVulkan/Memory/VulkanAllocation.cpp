#include "vkpch.h"
#include "VulkanAllocation.h"

#include <VoltVulkan/Common/VulkanCommon.h>

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Memory/Allocator.h>

#include <vma/VulkanMemoryAllocator.h>

namespace Volt::RHI
{
	void VulkanImageAllocation::Unmap()
	{
		vmaUnmapMemory(GraphicsContext::GetAllocator().GetHandle<VmaAllocator>(), m_allocation);
	}

	void* VulkanImageAllocation::GetResourceHandleInternal() const
	{
		return m_resource;
	}

	void* VulkanImageAllocation::MapInternal()
	{
		void* data = nullptr;
		VT_VK_CHECK(vmaMapMemory(GraphicsContext::GetAllocator().GetHandle<VmaAllocator>(), m_allocation, &data));
		return data;
	}

	void* VulkanImageAllocation::GetHandleImpl() const
	{
		return m_allocation;
	}

	void VulkanBufferAllocation::Unmap()
	{
		vmaUnmapMemory(GraphicsContext::GetAllocator().GetHandle<VmaAllocator>(), m_allocation);
	}

	void* VulkanBufferAllocation::GetResourceHandleInternal() const
	{
		return m_resource;
	}

	void* VulkanBufferAllocation::MapInternal()
	{
		void* data = nullptr;
		VT_VK_CHECK(vmaMapMemory(GraphicsContext::GetAllocator().GetHandle<VmaAllocator>(), m_allocation, &data));
		return data;
	}

	void* VulkanBufferAllocation::GetHandleImpl() const
	{
		return m_allocation;
	}

	void VulkanTransientBufferAllocation::Unmap()
	{
		auto device = GraphicsContext::GetDevice();
		vkUnmapMemory(device->GetHandle<VkDevice>(), m_memoryHandle);
	}
	
	void* VulkanTransientBufferAllocation::GetResourceHandleInternal() const
	{
		return m_resource;
	}
	
	void* VulkanTransientBufferAllocation::MapInternal()
	{
		auto device = GraphicsContext::GetDevice();

		void* mappedMemory = nullptr;
		VT_VK_CHECK(vkMapMemory(device->GetHandle<VkDevice>(), m_memoryHandle, m_allocationBlock.offset, m_allocationBlock.size, 0, &mappedMemory));

		return mappedMemory;
	}
	
	void* VulkanTransientBufferAllocation::GetHandleImpl() const
	{
		return m_memoryHandle;
	}
	
	void VulkanTransientImageAllocation::Unmap()
	{
		auto device = GraphicsContext::GetDevice();
		vkUnmapMemory(device->GetHandle<VkDevice>(), m_memoryHandle);
	}
	
	void* VulkanTransientImageAllocation::GetResourceHandleInternal() const
	{
		return m_resource;
	}
	
	void* VulkanTransientImageAllocation::MapInternal()
	{
		auto device = GraphicsContext::GetDevice();

		void* mappedMemory = nullptr;
		VT_VK_CHECK(vkMapMemory(device->GetHandle<VkDevice>(), m_memoryHandle, m_allocationBlock.offset, m_allocationBlock.size, 0, &mappedMemory));

		return mappedMemory;
	}
	
	void* VulkanTransientImageAllocation::GetHandleImpl() const
	{
		return m_memoryHandle;
	}
}
