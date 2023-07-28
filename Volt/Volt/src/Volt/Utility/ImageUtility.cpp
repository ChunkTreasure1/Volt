#include "vtpch.h"
#include "ImageUtility.h"


namespace Volt
{
	namespace Utility
	{
		void InsertImageMemoryBarriers(VkCommandBuffer commandBuffer, const std::vector<VkImageMemoryBarrier>& barriers, VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags)
		{
			vkCmdPipelineBarrier(commandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, (uint32_t)barriers.size(), barriers.data());
		}
	}
}
