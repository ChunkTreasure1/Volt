#pragma once

#include <vulkan/vulkan.h>

#define VT_GET_VULKAN_FUNCTION(functionName) functionName = (PFN_ ## functionName)vkGetInstanceProcAddr(instance, #functionName)

namespace Volt::RHI
{
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	inline PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	inline PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

	inline static void FindVulkanFunctions(VkInstance instance)
	{
		VT_GET_VULKAN_FUNCTION(vkSetDebugUtilsObjectNameEXT);
		VT_GET_VULKAN_FUNCTION(vkCmdBeginDebugUtilsLabelEXT);
		VT_GET_VULKAN_FUNCTION(vkCmdEndDebugUtilsLabelEXT);
	}
}
