#include "vkpch.h"
#include "VulkanTransientHeap.h"

#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Memory/MemoryUtility.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanTransientHeap::VulkanTransientHeap(const TransientHeapCreateInfo& info)
		: m_createInfo(info)
	{
		auto physicalDevice = GraphicsContext::GetPhysicalDevice();

		if ((info.flags & TransientHeapFlags::AllowBuffers) != TransientHeapFlags::None)
		{
			InitializeAsBufferHeap();
		}
		else if ((info.flags & TransientHeapFlags::AllowTextures) != TransientHeapFlags::None || (info.flags & TransientHeapFlags::AllowRenderTargets) != TransientHeapFlags::None)
		{
			InitializeAsImageHeap();
		}
	}

	VulkanTransientHeap::~VulkanTransientHeap()
	{
	}

	Ref<Allocation> VulkanTransientHeap::CreateBuffer(const TransientBufferCreateInfo& createInfo)
	{
		return Ref<Allocation>();
	}

	const bool VulkanTransientHeap::IsAllocationSupported(const uint64_t size, TransientHeapFlags heapFlags) const
	{
		return false;
	}

	void VulkanTransientHeap::InitializeAsBufferHeap()
	{
		auto device = GraphicsContext::GetDevice();
		auto physicalDevice = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>();

		constexpr VkBufferUsageFlags USAGE_FLAGS = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
			VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.size = m_createInfo.size;
		createInfo.usage = USAGE_FLAGS;

		VkDeviceBufferMemoryRequirements bufferMemReq{};
		bufferMemReq.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;

		VkMemoryRequirements2 memReq{};
		vkGetDeviceBufferMemoryRequirements(device->GetHandle<VkDevice>(), &bufferMemReq, &memReq);

		m_memoryRequirements.alignment = memReq.memoryRequirements.alignment;
		m_memoryRequirements.size = memReq.memoryRequirements.size;
		m_memoryRequirements.memoryTypeBits = memReq.memoryRequirements.memoryTypeBits;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = Utility::Align(m_memoryRequirements.size, m_memoryRequirements.alignment);
		allocInfo.memoryTypeIndex = physicalDevice->GetMemoryTypeIndex(m_memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory allocatedMemory = nullptr;
		vkAllocateMemory(device->GetHandle<VkDevice>(), &allocInfo, nullptr, &allocatedMemory);
	}

	void VulkanTransientHeap::InitializeAsImageHeap()
	{
	}
}
