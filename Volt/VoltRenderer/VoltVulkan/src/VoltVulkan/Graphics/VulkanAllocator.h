#pragma once

#include <VoltRHI/Core/Core.h>

#include <vma/VulkanMemoryAllocator.h>

namespace Volt
{
	class VulkanGraphicsDevice;

	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;
		VulkanAllocator(const std::string& tag);

		~VulkanAllocator();

		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkBuffer& outBuffer, std::string_view name = "");
		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaAllocationCreateFlags allocationFlags, VkBuffer& outBuffer, std::string_view name = "");
		VmaAllocation AllocateImage(VkImageCreateInfo bufferCreateInfo, VmaMemoryUsage memoryUsage, VkImage& outImage, std::string_view name = "");

		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);
		void DestroyImage(VkImage image, VmaAllocation allocation);

		template<typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* data = nullptr;
			vmaMapMemory(VulkanAllocatorVolt::GetAllocator(), allocation, (void**)&data);
			return data;
		}

		void UnmapMemory(VmaAllocation allocation);

		static void Initialize(Ref<VulkanGraphicsDevice> graphicsDevice);
		static void Shutdown();
		static void SetFrameIndex(const uint32_t index);

		static VmaAllocator& GetAllocator();

	private:
#ifdef VT_ENABLE_DEBUG_ALLOCATIONS
		uint64_t m_allocatedBytes = 0;
		uint64_t m_freedBytes = 0;
#endif
		std::string m_tag;
	};
}
