#pragma once

#include <VoltRHI/Core/RHICommon.h>

#include <vulkan/vulkan.h>

namespace Volt
{
	namespace Utility
	{
		inline static constexpr VkFormat VoltToVulkanFormat(Format format)
		{
			return static_cast<VkFormat>(format);
		}

		inline static constexpr Format VulkanToVoltFormat(VkFormat format)
		{
			return static_cast<Format>(format);
		}

		inline static constexpr VkPresentModeKHR VoltToVulkanPresentMode(PresentMode presentMode)
		{
			return static_cast<VkPresentModeKHR>(presentMode);
		}

		inline static constexpr VkColorSpaceKHR VoltToVulkanColorSpace(ColorSpace colorSpace)
		{
			return static_cast<VkColorSpaceKHR>(colorSpace);
		}


	}
}
