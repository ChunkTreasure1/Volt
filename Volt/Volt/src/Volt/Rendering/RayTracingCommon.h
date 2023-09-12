#pragma once

#include <vulkan/vulkan.h>

namespace Volt
{
	struct AccelerationStructure
	{
		VkAccelerationStructureKHR handle;
		uint64_t deviceAddress = 0;

		VkBuffer buffer;
	};

	struct ScratchBuffer
	{
		uint64_t deviceAddress = 0;
		VkBuffer handle = VK_NULL_HANDLE;
	};
}
