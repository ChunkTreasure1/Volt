#pragma once

#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>

struct VkPhysicalDevice_T;

namespace Volt::RHI
{
	struct PhysicalDeviceQueueFamilyIndices
	{
		int32_t graphicsFamilyQueueIndex = -1;
		int32_t computeFamilyQueueIndex = -1;
		int32_t transferFamilyQueueIndex = -1;
	};

	struct PhysicalDeviceMemoryProperties
	{
		struct MemoryType
		{
			uint32_t propertyFlags;
			uint32_t heapIndex;
			uint32_t index;
		};

		struct MemoryHeap
		{
			uint64_t size;
			uint32_t flags;
		};

		std::vector<MemoryType> memoryTypes;
		std::vector<MemoryHeap> memoryHeaps;
	};

	class VulkanPhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		VulkanPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo);
		~VulkanPhysicalGraphicsDevice() override;

		inline const PhysicalDeviceQueueFamilyIndices& GetQueueFamilies() const { return m_queueFamilyIndices; }

		const int32_t GetMemoryTypeIndex(const uint32_t reqMemoryTypeBits, const uint32_t requiredPropertyFlags);

	protected:
		void* GetHandleImpl() const override;

	private:
		void FindMemoryProperties();

		VkPhysicalDevice_T* m_physicalDevice = nullptr;

		PhysicalDeviceQueueFamilyIndices m_queueFamilyIndices{};
		PhysicalDeviceMemoryProperties m_deviceMemoryProperties;

		PhysicalDeviceCreateInfo m_createInfo{};
	};
}
