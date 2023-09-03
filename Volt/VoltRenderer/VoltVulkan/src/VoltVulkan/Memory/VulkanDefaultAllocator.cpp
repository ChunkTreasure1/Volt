#include "vkpch.h"
#include "VulkanDefaultAllocator.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Memory/VulkanAllocation.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>
#include <VoltRHI/Images/ImageUtility.h>

#include <VoltRHI/Core/Profiling.h>
#include <VoltRHI/Memory/MemoryPool.h>

#include <vma/VulkanMemoryAllocator.h>

namespace Volt::RHI
{
	namespace Utility
	{
		VkBufferUsageFlags GetVkBufferUsageFlags(BufferUsage usageFlags)
		{
			VkBufferUsageFlags result = 0;

			if ((usageFlags & BufferUsage::TransferSrc) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			}

			if ((usageFlags & BufferUsage::TransferDst) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			}

			if ((usageFlags & BufferUsage::UniformBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::StorageBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::IndexBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::VertexBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::IndirectBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			}

			return result;
		}

		VkImageUsageFlags GetVkImageUsageFlags(ImageUsage usageFlags, Format imageFormat)
		{
			VkImageUsageFlags result = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			if (usageFlags == ImageUsage::Attachment)
			{
				result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				
				if (Utility::IsDepthFormat(imageFormat))
				{
					result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				else
				{
					result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			}
			else if (usageFlags == ImageUsage::AttachmentStorage)
			{
				result |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

				if (Utility::IsDepthFormat(imageFormat))
				{
					result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				else
				{
					result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			}
			else if (usageFlags == ImageUsage::Texture)
			{
				result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			else if (usageFlags == ImageUsage::Storage)
			{
				result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			}

			return result;
		}
	}

	VulkanDefaultAllocator::VulkanDefaultAllocator()
	{
		VmaAllocatorCreateInfo info{};
		info.vulkanApiVersion = VK_API_VERSION_1_3;
		info.physicalDevice = GraphicsContext::GetPhysicalDevice()->GetHandle<VkPhysicalDevice>();
		info.device = GraphicsContext::GetDevice()->GetHandle<VkDevice>();
		info.instance = GraphicsContext::Get().GetHandle<VkInstance>();
		info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		VT_VK_CHECK(vmaCreateAllocator(&info, &m_allocator));
	}

	VulkanDefaultAllocator::~VulkanDefaultAllocator()
	{
		for (const auto& alloc : m_activeImageAllocations)
		{
			DestroyImage(alloc);
		}

		for (const auto& alloc : m_activeBufferAllocations)
		{
			DestroyBuffer(alloc);
		}

		vmaDestroyAllocator(m_allocator);
	}

	Ref<Allocation> VulkanDefaultAllocator::CreateBuffer(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.size = size;
		bufferInfo.usage = Utility::GetVkBufferUsageFlags(usage);

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

		Ref<VulkanBufferAllocation> allocation = CreateRefRHI<VulkanBufferAllocation>();
		VT_VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &allocCreateInfo, &allocation->m_resource, &allocation->m_allocation, &allocInfo));

		m_activeBufferAllocations.push_back(allocation);
		return allocation;
	}

	Ref<Allocation> VulkanDefaultAllocator::CreateImage(const ImageSpecification& imageSpecification, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageSpecification.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
		imageInfo.usage = Utility::GetVkImageUsageFlags(imageSpecification.usage, imageSpecification.format);
		imageInfo.extent.width = imageSpecification.width;
		imageInfo.extent.height = imageSpecification.height;
		imageInfo.extent.depth = imageSpecification.depth;
		imageInfo.mipLevels = imageSpecification.mips;
		imageInfo.arrayLayers = imageSpecification.layers;
		imageInfo.format = Utility::VoltToVulkanFormat(imageSpecification.format);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (imageSpecification.isCubeMap && imageSpecification.layers > 1)
		{
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		else if (imageSpecification.layers > 1)
		{
			imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
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
		allocCreateInfo.priority = 1.f;

		VmaAllocationInfo allocInfo{};

		Ref<VulkanImageAllocation> allocation = CreateRefRHI<VulkanImageAllocation>();
		VT_VK_CHECK(vmaCreateImage(m_allocator, &imageInfo, &allocCreateInfo, &allocation->m_resource, &allocation->m_allocation, &allocInfo));

		if (!imageSpecification.debugName.empty())
		{
			vmaSetAllocationName(m_allocator, allocation->m_allocation, imageSpecification.debugName.c_str());
		}

		m_activeImageAllocations.push_back(allocation);
		return allocation;
	}

	Ref<Allocation> VulkanDefaultAllocator::CreateImage(const ImageSpecification& imageSpecification, Ref<MemoryPool> pool, MemoryUsage memoryUsage)
	{
		VT_PROFILE_FUNCTION();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageSpecification.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
		imageInfo.usage = Utility::GetVkImageUsageFlags(imageSpecification.usage, imageSpecification.format);
		imageInfo.extent.width = imageSpecification.width;
		imageInfo.extent.height = imageSpecification.height;
		imageInfo.extent.depth = imageSpecification.depth;
		imageInfo.mipLevels = imageSpecification.mips;
		imageInfo.arrayLayers = imageSpecification.layers;
		imageInfo.format = Utility::VoltToVulkanFormat(imageSpecification.format);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (imageSpecification.isCubeMap && imageSpecification.layers > 1)
		{
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		else if (imageSpecification.layers > 1)
		{
			imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
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
		allocCreateInfo.priority = 1.f;
		allocCreateInfo.pool = pool->GetHandle<VmaPool>();

		VmaAllocationInfo allocInfo{};

		Ref<VulkanImageAllocation> allocation = CreateRefRHI<VulkanImageAllocation>();
		VT_VK_CHECK(vmaCreateImage(m_allocator, &imageInfo, &allocCreateInfo, &allocation->m_resource, &allocation->m_allocation, &allocInfo));

		if (!imageSpecification.debugName.empty())
		{
			vmaSetAllocationName(m_allocator, allocation->m_allocation, imageSpecification.debugName.c_str());
		}

		m_activeImageAllocations.push_back(allocation);
		return allocation;
	}

	void VulkanDefaultAllocator::DestroyBuffer(Ref<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		Ref<VulkanBufferAllocation> bufferAlloc = allocation->As<VulkanBufferAllocation>();
		vmaDestroyBuffer(m_allocator, bufferAlloc->m_resource, bufferAlloc->m_allocation);

		auto it = std::find(m_activeBufferAllocations.begin(), m_activeBufferAllocations.end(), allocation);
		if (it != m_activeBufferAllocations.end())
		{
			m_activeBufferAllocations.erase(it);
		}
	}

	void VulkanDefaultAllocator::DestroyImage(Ref<Allocation> allocation)
	{
		VT_PROFILE_FUNCTION();

		Ref<VulkanImageAllocation> imageAlloc = allocation->As<VulkanImageAllocation>();
		vmaDestroyImage(m_allocator, imageAlloc->m_resource, imageAlloc->m_allocation);

		auto it = std::find(m_activeImageAllocations.begin(), m_activeImageAllocations.end(), allocation);
		if (it != m_activeImageAllocations.end())
		{
			m_activeImageAllocations.erase(it);
		}
	}

	void* VulkanDefaultAllocator::GetHandleImpl() const
	{
		return m_allocator;
	}
}
