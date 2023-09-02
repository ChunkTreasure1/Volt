#include "vkpch.h"
#include "VulkanAllocation.h"

#include <VoltVulkan/Common/VulkanCommon.h>

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

	void* VulkanImageAllocation::GetHandleImpl()
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

	void* VulkanBufferAllocation::GetHandleImpl()
	{
		return m_allocation;
	}
}
