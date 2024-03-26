#include "vtpch.h"
#include "VulkanAllocator.h"

#include "Volt/Core/Graphics/GraphicsDevice.h"
#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	struct VulkanAllocatorData
	{
		VmaAllocator allocator;
		uint64_t totalFreedBytes = 0;
		uint64_t totalAllocatedBytes = 0;
	};

	static VulkanAllocatorData* s_allocatorData = nullptr;

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

	VulkanAllocator::VulkanAllocator(const std::string& tag)
		: myTag(tag)
	{
	}

	VulkanAllocator::~VulkanAllocator()
	{
#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		//if ((myAllocatedBytes != 0 || myFreedBytes != 0) && !myTag.empty())
		//{
		//	const auto [allocatedSize, allocatedSuffix] = Utility::GetClosestUnitSize(myAllocatedBytes);
		//	const auto [freedSize, freedSuffix] = Utility::GetClosestUnitSize(myFreedBytes);

		//	VT_CORE_TRACE("[VulkanAllocator] {0} allocated {1}{2} and freed {3}{4}!", myTag.c_str(), allocatedSize, allocatedSuffix, freedSize, freedSuffix);
		//}
#endif
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

		myAllocatedBytes += (uint64_t)allocInfo.size;
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

		myAllocatedBytes += (uint64_t)allocInfo.size;
#endif

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImage(VkImageCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkImage& outImage, std::string_view name)
	{
		VT_PROFILE_FUNCTION();

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

		myAllocatedBytes += (uint64_t)allocInfo.size;
#endif

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImageInPool(VkImageCreateInfo bufferCreateInfo, VmaMemoryUsage, VkImage& outImage, VmaPool pool, std::string_view name)
	{
		VT_PROFILE_FUNCTION();

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.pool = pool;

		VmaAllocation allocation;
		VT_VK_CHECK(vmaCreateImage(s_allocatorData->allocator, &bufferCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr));

		if (!name.empty())
		{
			VT_PROFILE_SCOPE("Set allocation name");
			vmaSetAllocationName(s_allocatorData->allocator, allocation, name.data());
		}

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);

		s_allocatorData->totalAllocatedBytes += allocInfo.size;

		myAllocatedBytes += (uint64_t)allocInfo.size;
#endif

		return allocation;
	}

	const size_t VulkanAllocator::GetAllocationSize(VmaAllocation allocation)
	{
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);
		return allocInfo.size;
	}

	void VulkanAllocator::Free(VmaAllocation allocation)
	{
		VT_CORE_ASSERT(allocation, "Unable to free null allocation!");

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);
		s_allocatorData->totalFreedBytes += allocInfo.size;

		myFreedBytes += (uint64_t)allocInfo.size;
#endif
		vmaFreeMemory(s_allocatorData->allocator, allocation);
	}

	void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
		VT_CORE_ASSERT(buffer, "Unable to destroy null buffer!");
		VT_CORE_ASSERT(allocation, "Unable to free null allocation!");

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);
		s_allocatorData->totalFreedBytes += allocInfo.size;

		myFreedBytes += (uint64_t)allocInfo.size;
#endif

		vmaDestroyBuffer(s_allocatorData->allocator, buffer, allocation);
	}

	void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		VT_PROFILE_FUNCTION();

		VT_CORE_ASSERT(image, "Unable to destroy null image!");
		VT_CORE_ASSERT(allocation, "Unable to free null allocation!");

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_allocatorData->allocator, allocation, &allocInfo);
		s_allocatorData->totalFreedBytes += allocInfo.size;

		myFreedBytes += (uint64_t)allocInfo.size;
#endif

		vmaDestroyImage(s_allocatorData->allocator, image, allocation);
	}

	void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_allocatorData->allocator, allocation);
	}

	void VulkanAllocator::Initialize(Ref<GraphicsDevice> graphicsDevice)
	{
		s_allocatorData = new VulkanAllocatorData();

		auto physDevicePtr = graphicsDevice->GetPhysicalDevice();

		VmaAllocatorCreateInfo info{};
		info.vulkanApiVersion = VK_API_VERSION_1_3;
		info.physicalDevice = physDevicePtr->GetHandle();
		info.device = graphicsDevice->GetHandle();
		info.instance = GraphicsContext::Get().GetInstance();
		info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		VT_VK_CHECK(vmaCreateAllocator(&info, &s_allocatorData->allocator));
	}

	void VulkanAllocator::Shutdown()
	{
		VT_CORE_ASSERT(s_allocatorData->totalAllocatedBytes - s_allocatorData->totalFreedBytes == 0, "Some data has not been freed! This will cause a memory leak!");
		VT_CORE_ASSERT(s_allocatorData->allocator, "Unable to delete allocator as it does not exist!");

		vmaDestroyAllocator(s_allocatorData->allocator);
		s_allocatorData->allocator = nullptr;

		VT_CORE_ASSERT(s_allocatorData, "Unable to delete allocator data as it does not exist!");

		delete s_allocatorData;
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
