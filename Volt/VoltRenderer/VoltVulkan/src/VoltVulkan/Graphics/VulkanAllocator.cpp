#include "vkpch.h"
#include "VulkanAllocator.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"

namespace Volt::RHI
{
	namespace Utility
	{
		inline const std::pair<float, std::string> GetClosestUnitSize(const uint64_t bytes)
		{
			constexpr uint64_t KILOBYTE = 1024;
			constexpr uint64_t MEGABYTE = 1024 * 1024;
			constexpr uint64_t GIGABYTE = 1024 * 1024 * 1024;

			if (bytes > GIGABYTE)
			{
				return { (float)bytes / (float)GIGABYTE, "Gb" };
			}

			if (bytes > MEGABYTE)
			{
				return { (float)bytes / (float)MEGABYTE, "Mb" };
			}

			if (bytes > KILOBYTE)
			{
				return { (float)bytes / (float)KILOBYTE, "Kb" };
			}

			return { (float)bytes, "bytes" };
		}
	}

	struct VulkanAllocatorData
	{
		VmaAllocator allocator;
		uint64_t totalFreedBytes = 0;
		uint64_t totalAllocatedBytes = 0;
	};

	static Scope<VulkanAllocatorData> s_allocatorData = nullptr;


	VulkanAllocator::VulkanAllocator(const std::string& tag)
		: m_tag(tag)
	{
	}

	VulkanAllocator::~VulkanAllocator()
	{
	}

	VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkBuffer& outBuffer, std::string_view name)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memoryUsage;

		VmaAllocation allocation;
		vmaCreateBuffer(s_allocatorData->allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr);

		if (!name.empty())
		{
			vmaSetAllocationName(s_allocatorData->allocator, allocation, name.data());
		}

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);

		s_allocatorData->totalAllocatedBytes += allocInfo.size;
		m_allocatedBytes += (uint64_t)allocInfo.size;
#endif

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaAllocationCreateFlags allocationFlags, VkBuffer& outBuffer, std::string_view name)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = allocationFlags;

		VmaAllocation allocation;
		vmaCreateBuffer(s_allocatorData->allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr);

		if (!name.empty())
		{
			vmaSetAllocationName(s_allocatorData->allocator, allocation, name.data());
		}

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);

		s_allocatorData->totalAllocatedBytes += allocInfo.size;
		m_allocatedBytes += (uint64_t)allocInfo.size;
#endif

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImage(VkImageCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkImage& outImage, std::string_view name)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memoryUsage;

		VmaAllocation allocation;
		VT_VK_CHECK(vmaCreateImage(s_allocatorData->allocator, &bufferCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr));

		if (!name.empty())
		{
			vmaSetAllocationName(s_allocatorData->allocator, allocation, name.data());
		}

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);

		s_allocatorData->totalAllocatedBytes += allocInfo.size;
		m_allocatedBytes += (uint64_t)allocInfo.size;
#endif

		return allocation;
	}

	void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
		assert(buffer && "Unable to destroy null buffer!");
		assert(allocation && "Unable to free null allocation!");

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);
		s_allocatorData->totalFreedBytes += allocInfo.size;

		m_freedBytes += (uint64_t)allocInfo.size;
#endif

		vmaDestroyBuffer(s_allocatorData->allocator, buffer, allocation);
	}

	void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		assert(image && "Unable to destroy null image!");
		assert(allocation && "Unable to free null allocation!");

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);
		s_allocatorData->totalFreedBytes += allocInfo.size;

		m_freedBytes += (uint64_t)allocInfo.size;
#endif

		vmaDestroyImage(s_allocatorData->allocator, image, allocation);
	}

	void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_allocatorData->allocator, allocation);
	}

	void VulkanAllocator::Initialize(Ref<VulkanGraphicsDevice> graphicsDevice)
	{
		s_allocatorData = CreateScopeRHI<VulkanAllocatorData>();

		auto physDevicePtr = graphicsDevice->GetPhysicalDevice().lock();

		VmaAllocatorCreateInfo info{};
		info.vulkanApiVersion = VK_API_VERSION_1_3;
		info.physicalDevice = physDevicePtr->GetHandle<VkPhysicalDevice>();
		info.device = graphicsDevice->GetHandle<VkDevice>();
		info.instance = GraphicsContext::Get().GetHandle<VkInstance>();
		info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		VT_VK_CHECK(vmaCreateAllocator(&info, &s_allocatorData->allocator));
	}

	void VulkanAllocator::Shutdown()
	{
		const size_t allocDiff = s_allocatorData->totalAllocatedBytes - s_allocatorData->totalFreedBytes;

		assert(allocDiff == 0 && "Some data has not been freed! This will cause a memory leak!");
		assert(s_allocatorData->allocator && "Unable to delete allocator as it does not exist!");

		vmaDestroyAllocator(s_allocatorData->allocator);
		s_allocatorData->allocator = nullptr;

		assert(s_allocatorData && "Unable to delete allocator data as it does not exist!");
		s_allocatorData = nullptr;
	}

	void VulkanAllocator::SetFrameIndex(const uint32_t index)
	{
		vmaSetCurrentFrameIndex(s_allocatorData->allocator, index);
	}

	VmaAllocator& VulkanAllocator::GetAllocator()
	{
		return s_allocatorData->allocator;
	}
}
