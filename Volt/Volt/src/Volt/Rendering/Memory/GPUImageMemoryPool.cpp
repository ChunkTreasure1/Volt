#include "vtpch.h"
#include "GPUImageMemoryPool.h"

namespace Volt
{
	GPUImageMemoryPool::GPUImageMemoryPool(size_t poolSize, VkImageUsageFlags supportedUsages)
		: mySize(poolSize), mySupportedUsages(supportedUsages)
	{
		Invalidate();
	}

	GPUImageMemoryPool::~GPUImageMemoryPool()
	{
		Release();
	}

	void GPUImageMemoryPool::Invalidate()
	{
		VkImageCreateInfo exampleImageCreateInfo{};
		exampleImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		exampleImageCreateInfo.usage = mySupportedUsages;
		exampleImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		exampleImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		exampleImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo sampleAllocCreateInfo = {};
		sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		uint32_t memTypeIndex = 0;
		vmaFindMemoryTypeIndexForImageInfo(VulkanAllocator::GetAllocator(), &exampleImageCreateInfo, &sampleAllocCreateInfo, &memTypeIndex);

		VmaPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.memoryTypeIndex = memTypeIndex;
		poolCreateInfo.blockSize = mySize;
		poolCreateInfo.maxBlockCount = 1;
		poolCreateInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;

		VT_VK_CHECK(vmaCreatePool(VulkanAllocator::GetAllocator(), &poolCreateInfo, &myPool));
	}
	
	void GPUImageMemoryPool::Release()
	{
		vmaDestroyPool(VulkanAllocator::GetAllocator(), myPool);
	}
}
