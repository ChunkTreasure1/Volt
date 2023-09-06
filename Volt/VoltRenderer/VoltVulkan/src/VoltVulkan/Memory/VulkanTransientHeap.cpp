#include "vkpch.h"
#include "VulkanTransientHeap.h"

#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Memory/VulkanAllocation.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Memory/MemoryUtility.h>
#include <VoltRHI/Core/Profiling.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanTransientHeap::VulkanTransientHeap(const TransientHeapCreateInfo& info)
		: m_createInfo(info)
	{
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
		auto device = GraphicsContext::GetDevice();

		for (auto& page : m_pageAllocations)
		{
			if (page.handle)
			{
				vkFreeMemory(device->GetHandle<VkDevice>(), page.handle, nullptr);
				page.handle = nullptr;
			}
		}
	}

	Ref<Allocation> VulkanTransientHeap::CreateBuffer(const TransientBufferCreateInfo& createInfo)
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();

		auto [pageIndex, blockAlloc] = FindNextAvailableBlock(createInfo.size);

		if (blockAlloc.size == 0)
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanTransientHeap]", "Unable to find available allocation block for buffer allocation of size {0}!", createInfo.size);
			return nullptr;
		}

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.size = createInfo.size;
		bufferInfo.usage = Utility::GetVkBufferUsageFlags(createInfo.usage);

		const auto& page = m_pageAllocations.at(pageIndex);

		VkBuffer buffer;
		vkCreateBuffer(device->GetHandle<VkDevice>(), &bufferInfo, nullptr, &buffer);
		vkBindBufferMemory(device->GetHandle<VkDevice>(), buffer, page.handle, blockAlloc.offset);

		Ref<VulkanTransientBufferAllocation> bufferAlloc = CreateRefRHI<VulkanTransientBufferAllocation>();
		bufferAlloc->m_memoryHandle = page.handle;
		bufferAlloc->m_resource = buffer;
		bufferAlloc->m_allocationBlock = blockAlloc;
		bufferAlloc->m_heapId = m_heapId;

		return bufferAlloc;
	}

	Ref<Allocation> VulkanTransientHeap::CreateImage(const TransientImageCreateInfo& createInfo)
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		const auto& imageSpecification = createInfo.imageSpecification;

		const VkImageCreateInfo imageInfo = Utility::GetVkImageCreateInfo(imageSpecification);

		auto [pageIndex, blockAlloc] = FindNextAvailableBlock(createInfo.size);

		if (blockAlloc.size == 0)
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanTransientHeap]", "Unable to find available allocation block for buffer allocation of size {0}!", createInfo.size);
			return nullptr;
		}

		const auto& page = m_pageAllocations.at(pageIndex);
		
		VkImage image;
		vkCreateImage(device->GetHandle<VkDevice>(), &imageInfo, nullptr, &image);
		vkBindImageMemory(device->GetHandle<VkDevice>(), image, page.handle, blockAlloc.offset);

		Ref<VulkanTransientImageAllocation> imageAlloc = CreateRefRHI<VulkanTransientImageAllocation>();
		imageAlloc->m_memoryHandle = page.handle;
		imageAlloc->m_resource = image;
		imageAlloc->m_allocationBlock = blockAlloc;
		imageAlloc->m_heapId = m_heapId;

		return imageAlloc;
	}

	void VulkanTransientHeap::ForfeitBuffer(Ref<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		Ref<VulkanTransientBufferAllocation> bufferAlloc = std::reinterpret_pointer_cast<VulkanTransientBufferAllocation>(allocation);
		if (!bufferAlloc)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		vkDestroyBuffer(device->GetHandle<VkDevice>(), bufferAlloc->m_resource, nullptr);
	
		AllocationBlock allocBlock = bufferAlloc->m_allocationBlock;
		ForfeitAllocationBlock(allocBlock);
	}

	void VulkanTransientHeap::ForfeitImage(Ref<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		Ref<VulkanTransientImageAllocation> imageAlloc = std::reinterpret_pointer_cast<VulkanTransientImageAllocation>(allocation);
		if (!imageAlloc)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		vkDestroyImage(device->GetHandle<VkDevice>(), imageAlloc->m_resource, nullptr);

		AllocationBlock allocBlock = imageAlloc->m_allocationBlock;
		ForfeitAllocationBlock(allocBlock);
	}

	const bool VulkanTransientHeap::IsAllocationSupported(const uint64_t size, TransientHeapFlags heapFlags) const
	{
		if ((m_createInfo.flags & heapFlags) == TransientHeapFlags::None)
		{
			return false;
		}

		for (const auto& page : m_pageAllocations)
		{
			if (page.GetRemainingSize() >= size)
			{
				return true;
			}
		}

		return false;
	}

	void* VulkanTransientHeap::GetHandleImpl() const
	{
		return nullptr;
	}

	std::pair<uint32_t, AllocationBlock> VulkanTransientHeap::FindNextAvailableBlock(const uint64_t size)
	{
		PageAllocation* pageAllocation = nullptr;
		uint32_t pageIndex = 0;

		for (auto& page : m_pageAllocations)
		{
			if (page.GetRemainingSize() >= size)
			{
				pageAllocation = &page;
				break;
			}

			pageIndex++;
		}

		if (!pageAllocation)
		{
			return { 0, { 0, 0 } };
		}

		int32_t blockIndex = -1;

		for (int32_t i = 0; const auto & availableBlocks : pageAllocation->availableBlocks)
		{
			if (availableBlocks.size >= size)
			{
				blockIndex = i;
				break;
			}

			i++;
		}

		AllocationBlock resultBlock{};

		if (blockIndex != -1)
		{
			AllocationBlock oldBlock = pageAllocation->availableBlocks.at(blockIndex);
			pageAllocation->availableBlocks.erase(pageAllocation->availableBlocks.begin() + blockIndex);

			// If we didn't use the whole block, create a new available block with the smaller allocation
			if (oldBlock.size > size)
			{
				auto& newBlock = pageAllocation->availableBlocks.emplace_back();
				newBlock.size = oldBlock.size - size;
				newBlock.offset = oldBlock.offset + size;
			}

			resultBlock.offset = oldBlock.offset;
			resultBlock.size = size;
		}
		else
		{
			resultBlock.offset = pageAllocation->tail;
			resultBlock.size = size;

			pageAllocation->tail += size;
		}

		resultBlock.pageId = pageIndex;
		pageAllocation->usedSize += size;

		return { pageIndex, resultBlock };
	}

	void VulkanTransientHeap::ForfeitAllocationBlock(const AllocationBlock& allocBlock)
	{
		auto& pageAllocation = m_pageAllocations.at(allocBlock.pageId);
	
		uint64_t finalOffset = allocBlock.offset;
		uint64_t finalSize = allocBlock.size;

		std::vector<size_t> blocksToMerge;

		for (size_t index = 0; const auto& block : pageAllocation.availableBlocks)
		{
			if (block.offset + block.size == finalOffset)
			{
				finalOffset = block.offset;
				finalSize += block.size;
				blocksToMerge.emplace_back(index);
			}
			else if (block.offset == finalOffset + finalSize)
			{
				finalSize += block.size;
				blocksToMerge.emplace_back(index);
			}

			index++;
		}

		for (int32_t i = static_cast<int32_t>(blocksToMerge.size()) - 1; i >= 0; i--)
		{
			pageAllocation.availableBlocks.erase(pageAllocation.availableBlocks.begin() + blocksToMerge.at(i));
			blocksToMerge.erase(blocksToMerge.begin() + i);
		}

		uint64_t finalEndOffset = finalOffset + finalSize;
		if (finalEndOffset == pageAllocation.tail)
		{
			pageAllocation.tail = finalOffset;
		}
		else
		{
			pageAllocation.availableBlocks.emplace_back(finalSize, finalOffset, 0);
		}
	
		pageAllocation.usedSize -= allocBlock.size;
	}

	void VulkanTransientHeap::InitializeAsBufferHeap()
	{
		auto device = GraphicsContext::GetDevice();
		auto& physicalDevice = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>();

		constexpr VkBufferUsageFlags USAGE_FLAGS = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
			VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.size = m_createInfo.pageSize;
		createInfo.usage = USAGE_FLAGS;

		VkDeviceBufferMemoryRequirements bufferMemReq{};
		bufferMemReq.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;
		bufferMemReq.pNext = nullptr;
		bufferMemReq.pCreateInfo = &createInfo;

		VkMemoryRequirements2 memReq{};
		memReq.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memReq.pNext = nullptr;

		vkGetDeviceBufferMemoryRequirements(device->GetHandle<VkDevice>(), &bufferMemReq, &memReq);

		m_memoryRequirements.alignment = memReq.memoryRequirements.alignment;
		m_memoryRequirements.size = memReq.memoryRequirements.size;
		m_memoryRequirements.memoryTypeBits = memReq.memoryRequirements.memoryTypeBits;

		m_memoryRequirements.size = Utility::Align(m_memoryRequirements.size, m_memoryRequirements.alignment);

		int32_t memoryTypeIndex = physicalDevice.GetMemoryTypeIndex(m_memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (memoryTypeIndex == -1)
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanTransientHeap]", "Unable to find memory type index from bits {0}!", m_memoryRequirements.memoryTypeBits);
			return;
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = m_memoryRequirements.size;
		allocInfo.memoryTypeIndex = static_cast<uint32_t>(memoryTypeIndex);

		for (uint32_t i = 0; i < MAX_PAGE_COUNT; i++)
		{
			m_pageAllocations[i].size = allocInfo.allocationSize;
			m_pageAllocations[i].alignment = m_memoryRequirements.alignment;
			vkAllocateMemory(device->GetHandle<VkDevice>(), &allocInfo, nullptr, &m_pageAllocations[i].handle);
		}
	}

	void VulkanTransientHeap::InitializeAsImageHeap()
	{
		auto device = GraphicsContext::GetDevice();
		auto& physicalDevice = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>();

		constexpr VkImageUsageFlags USAGE_FLAGS = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.flags = 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = 1;
		imageInfo.extent.height = 1;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = USAGE_FLAGS;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0;
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkDeviceImageMemoryRequirements imageMemReq{};
		imageMemReq.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
		imageMemReq.pNext = nullptr;
		imageMemReq.pCreateInfo = &imageInfo;

		VkMemoryRequirements2 memReq{};
		memReq.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memReq.pNext = nullptr;

		vkGetDeviceImageMemoryRequirements(device->GetHandle<VkDevice>(), &imageMemReq, &memReq);

		m_memoryRequirements.size = m_createInfo.pageSize;
		m_memoryRequirements.alignment = memReq.memoryRequirements.alignment;
		m_memoryRequirements.memoryTypeBits = memReq.memoryRequirements.memoryTypeBits;

		m_memoryRequirements.size = Utility::Align(m_createInfo.pageSize, m_memoryRequirements.alignment);

		int32_t memoryTypeIndex = physicalDevice.GetMemoryTypeIndex(m_memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (memoryTypeIndex == -1)
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanTransientHeap]", "Unable to find memory type index from bits {0}!", m_memoryRequirements.memoryTypeBits);
			return;
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = m_memoryRequirements.size;
		allocInfo.memoryTypeIndex = static_cast<uint32_t>(memoryTypeIndex);

		for (uint32_t i = 0; i < MAX_PAGE_COUNT; i++)
		{
			m_pageAllocations[i].size = allocInfo.allocationSize;
			m_pageAllocations[i].alignment = m_memoryRequirements.alignment;
			vkAllocateMemory(device->GetHandle<VkDevice>(), &allocInfo, nullptr, &m_pageAllocations[i].handle);
		}
	}
}
