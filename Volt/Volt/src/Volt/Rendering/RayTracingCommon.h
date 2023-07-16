#pragma once

#include "Volt/Core/Graphics/VulkanAllocatorVolt.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	struct AccelerationStructure
	{
		VkAccelerationStructureKHR handle;
		uint64_t deviceAddress = 0;

		VmaAllocation allocation;
		VkBuffer buffer;
	};

	struct ScratchBuffer
	{
		uint64_t deviceAddress = 0;
		VkBuffer handle = VK_NULL_HANDLE;
		VmaAllocation allocation;
	};
}
