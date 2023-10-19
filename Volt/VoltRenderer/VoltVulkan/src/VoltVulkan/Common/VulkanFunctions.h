#pragma once

#include <vulkan/vulkan.h>

#define VT_GET_VULKAN_FUNCTION(functionName) functionName = (PFN_ ## functionName)vkGetInstanceProcAddr(instance, #functionName)

namespace Volt::RHI
{
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	inline PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	inline PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	
	// Descriptor Buffers
	inline PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT;
	inline PFN_vkGetDescriptorEXT vkGetDescriptorEXT;
	inline PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT;
	inline PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT;
	inline PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT;

	inline static void FindVulkanFunctions(VkInstance instance)
	{
		VT_GET_VULKAN_FUNCTION(vkSetDebugUtilsObjectNameEXT);
		VT_GET_VULKAN_FUNCTION(vkCmdBeginDebugUtilsLabelEXT);
		VT_GET_VULKAN_FUNCTION(vkCmdEndDebugUtilsLabelEXT);

		// Descriptor Buffers
		VT_GET_VULKAN_FUNCTION(vkGetDescriptorSetLayoutSizeEXT);
		VT_GET_VULKAN_FUNCTION(vkGetDescriptorSetLayoutBindingOffsetEXT);
		VT_GET_VULKAN_FUNCTION(vkGetDescriptorEXT);
		VT_GET_VULKAN_FUNCTION(vkCmdBindDescriptorBuffersEXT);
		VT_GET_VULKAN_FUNCTION(vkCmdSetDescriptorBufferOffsetsEXT);
	}
}
