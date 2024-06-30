#include "vkpch.h"
#include "VulkanAllocation.h"

#include <VoltVulkan/Common/VulkanCommon.h>

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Memory/Allocator.h>

#include <vma/VulkanMemoryAllocator.h>

namespace Volt::RHI
{
	VulkanImageAllocation::VulkanImageAllocation(const size_t hash)
	{
		m_allocationHash = hash;
	}

	void VulkanImageAllocation::Unmap()
	{
		vmaUnmapMemory(GraphicsContext::GetDefaultAllocator()->GetHandle<VmaAllocator>(), m_allocation);
	}

	const uint64_t VulkanImageAllocation::GetDeviceAddress() const
	{
		return 0;
	}

	void* VulkanImageAllocation::GetResourceHandleInternal() const
	{
		return m_resource;
	}

	void* VulkanImageAllocation::MapInternal()
	{
		void* data = nullptr;
		VT_VK_CHECK(vmaMapMemory(GraphicsContext::GetDefaultAllocator()->GetHandle<VmaAllocator>(), m_allocation, &data));
		return data;
	}

	void* VulkanImageAllocation::GetHandleImpl() const
	{
		return m_allocation;
	}

	VulkanBufferAllocation::VulkanBufferAllocation(const size_t hash)
	{
		m_allocationHash = hash;
	}

	void VulkanBufferAllocation::Unmap()
	{
		vmaUnmapMemory(GraphicsContext::GetDefaultAllocator()->GetHandle<VmaAllocator>(), m_allocation);
	}

	const uint64_t VulkanBufferAllocation::GetDeviceAddress() const
	{
		VkBufferDeviceAddressInfo bufferAddressInfo{};
		bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferAddressInfo.pNext = nullptr;
		bufferAddressInfo.buffer = m_resource;
		return vkGetBufferDeviceAddress(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &bufferAddressInfo);
	}

	void* VulkanBufferAllocation::GetResourceHandleInternal() const
	{
		return m_resource;
	}

	void* VulkanBufferAllocation::MapInternal()
	{
		void* data = nullptr;
		VT_VK_CHECK(vmaMapMemory(GraphicsContext::GetDefaultAllocator()->GetHandle<VmaAllocator>(), m_allocation, &data));
		return data;
	}

	void* VulkanBufferAllocation::GetHandleImpl() const
	{
		return m_allocation;
	}

	VulkanTransientBufferAllocation::VulkanTransientBufferAllocation(const size_t hash)
	{
		m_allocationHash = hash;
	}

	void VulkanTransientBufferAllocation::Unmap()
	{
		auto device = GraphicsContext::GetDevice();
		vkUnmapMemory(device->GetHandle<VkDevice>(), m_memoryHandle);
	}

	const uint64_t VulkanTransientBufferAllocation::GetDeviceAddress() const
	{
		VkBufferDeviceAddressInfo bufferAddressInfo{};
		bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferAddressInfo.pNext = nullptr;
		bufferAddressInfo.buffer = m_resource;
		return vkGetBufferDeviceAddress(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &bufferAddressInfo);
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
	
	VulkanTransientImageAllocation::VulkanTransientImageAllocation(const size_t hash)
	{
		m_allocationHash = hash;
	}

	void VulkanTransientImageAllocation::Unmap()
	{
		auto device = GraphicsContext::GetDevice();
		vkUnmapMemory(device->GetHandle<VkDevice>(), m_memoryHandle);
	}

	const uint64_t VulkanTransientImageAllocation::GetDeviceAddress() const
	{
		return 0;
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
