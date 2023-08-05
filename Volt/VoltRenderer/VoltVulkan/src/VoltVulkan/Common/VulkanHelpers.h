#pragma once

#include <VoltRHI/Core/RHICommon.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
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
		
		inline static constexpr VkShaderStageFlagBits VoltToVulkanShaderStage(ShaderStage stage)
		{
			return static_cast<VkShaderStageFlagBits>(stage);
		}
	}
}
