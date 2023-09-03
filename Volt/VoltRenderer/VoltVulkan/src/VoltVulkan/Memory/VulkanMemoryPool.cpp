#include "vkpch.h"
#include "VulkanMemoryPool.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Memory/Allocator.h>

#include <vma/VulkanMemoryAllocator.h>
#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanMemoryPool::VulkanMemoryPool(MemoryUsage memoryUsage)
	{
		VkImageCreateInfo exampleImageCreateInfo{};
		exampleImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		exampleImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		exampleImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		exampleImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		exampleImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo sampleAllocCreateInfo = {};
		sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		auto& allocator = GraphicsContext::GetAllocator();

		uint32_t memTypeIndex = 0;
		vmaFindMemoryTypeIndexForImageInfo(allocator.GetHandle<VmaAllocator>(), &exampleImageCreateInfo, &sampleAllocCreateInfo, &memTypeIndex);

		VmaPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.memoryTypeIndex = memTypeIndex;
		poolCreateInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
		poolCreateInfo.blockSize = 128ull * 1024 * 1024;
		poolCreateInfo.maxBlockCount = 0;

		VT_VK_CHECK(vmaCreatePool(allocator.GetHandle<VmaAllocator>(), &poolCreateInfo, &m_pool));
	}

	VulkanMemoryPool::~VulkanMemoryPool()
	{
		vmaDestroyPool(GraphicsContext::GetAllocator().GetHandle<VmaAllocator>(), m_pool);
		m_pool = nullptr;
	}

	void* VulkanMemoryPool::GetHandleImpl() const
	{
		return m_pool;
	}
}
