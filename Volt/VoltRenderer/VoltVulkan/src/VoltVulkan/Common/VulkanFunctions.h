#pragma once

#include <vulkan/vulkan.h>

namespace Volt
{
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectName;

	inline static void FindVulkanFunctions(VkInstance instance)
	{
		vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
	}
}
