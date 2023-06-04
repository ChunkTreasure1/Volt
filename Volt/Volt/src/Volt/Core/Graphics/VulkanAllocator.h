#pragma once

#include "Volt/Core/Base.h"

#include <vma/VulkanMemoryAllocator.h>

namespace Volt
{
	class GraphicsDevice;
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;
		VulkanAllocator(const std::string& tag);

		~VulkanAllocator();

		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkBuffer& outBuffer, std::string_view name = "");
		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaAllocationCreateFlags allocationFlags, VkBuffer& outBuffer, std::string_view name = "");
		VmaAllocation AllocateImage(VkImageCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkImage& outImage, std::string_view name = "");

		VmaAllocation AllocateImageInPool(VkImageCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkImage& outImage, VmaPool pool, std::string_view name = "");

		void Free(VmaAllocation allocation);
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);
		void DestroyImage(VkImage image, VmaAllocation allocation);

		template<typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* data = nullptr;
			vmaMapMemory(VulkanAllocator::GetAllocator(), allocation, (void**)&data);
			return data;
		}

		void UnmapMemory(VmaAllocation allocation);

		static void Initialize(Ref<GraphicsDevice> graphicsDevice);
		static void Shutdown();
		static void SetFrameIndex(const uint32_t index);

		static VmaAllocator& GetAllocator();
	private:

#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		uint64_t myAllocatedBytes = 0;
		uint64_t myFreedBytes = 0;
#endif
		std::string myTag;
	};
}
