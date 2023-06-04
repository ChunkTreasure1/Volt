#pragma once

#include <vulkan/vulkan.h>

namespace Volt
{
	struct ImageBarrierInfo
	{
		VkPipelineStageFlags2 srcStageMask = 0;
		VkAccessFlags2 srcAccessMask = 0;

		VkPipelineStageFlags2 dstStageMask = 0;
		VkAccessFlags2 dstAccessMask = 0;

		VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageSubresourceRange subresourceRange{};
	};

	struct BufferBarrierInfo
	{
		VkPipelineStageFlags2 srcStageMask = 0;
		VkAccessFlags2 srcAccessMask = 0;

		VkPipelineStageFlags2 dstStageMask = 0;
		VkAccessFlags2 dstAccessMask = 0;

		VkDeviceSize offset = 0;
		VkDeviceSize size = VK_WHOLE_SIZE;
	};

	struct ExecutionBarrierInfo
	{
		VkPipelineStageFlags2 srcStageMask = 0;
		VkAccessFlags2 srcAccessMask = 0;

		VkPipelineStageFlags2 dstStageMask = 0;
		VkAccessFlags2 dstAccessMask = 0;
	};
}
