#pragma once

#include <vulkan/vulkan.h>

#define VT_GET_VULKAN_FUNCTION(functionName) functionName = (PFN_ ## functionName)vkGetInstanceProcAddr(instance, #functionName)

namespace Volt::RHI
{
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

	inline static void FindVulkanFunctions(VkInstance instance)
	{
		VT_GET_VULKAN_FUNCTION(vkSetDebugUtilsObjectNameEXT);
	}
}