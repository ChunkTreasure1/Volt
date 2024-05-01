#include "vkpch.h"
#include "VulkanHelpers.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Core/Profiling.h>

namespace Volt::RHI::Utility
{
	const VkImageCreateInfo GetVkImageCreateInfo(const ImageSpecification& imageSpecification)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageSpecification.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
		imageInfo.usage = Utility::GetVkImageUsageFlags(imageSpecification.usage, imageSpecification.format);
		imageInfo.extent.width = imageSpecification.width;
		imageInfo.extent.height = imageSpecification.height;
		imageInfo.extent.depth = imageSpecification.depth;
		imageInfo.mipLevels = imageSpecification.mips;
		imageInfo.arrayLayers = imageSpecification.layers;
		imageInfo.format = Utility::VoltToVulkanFormat(imageSpecification.format);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (imageSpecification.isCubeMap && imageSpecification.layers > 1)
		{
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		else if (imageSpecification.layers > 1 && imageSpecification.depth > 1)
		{
			imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
		}

		return imageInfo;
	}

	const MemoryRequirement GetImageRequirement(const VkImageCreateInfo& imageCreateInfo)
	{
		VT_PROFILE_FUNCTION();

		VkDeviceImageMemoryRequirements imageMemReq{};
		imageMemReq.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
		imageMemReq.pNext = nullptr;
		imageMemReq.pCreateInfo = &imageCreateInfo;

		VkMemoryRequirements2 memReq{};
		memReq.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memReq.pNext = nullptr;

		auto device = GraphicsContext::GetDevice();

		vkGetDeviceImageMemoryRequirements(device->GetHandle<VkDevice>(), &imageMemReq, &memReq);

		MemoryRequirement result{};
		result.size = memReq.memoryRequirements.size;
		result.alignment = memReq.memoryRequirements.alignment;
		result.memoryTypeBits = memReq.memoryRequirements.memoryTypeBits;

		return result;
	}

	const ImageLayout GetImageLayoutFromVkImageLayout(VkImageLayout layout)
	{
		switch (layout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED: return ImageLayout::Undefined;
			case VK_IMAGE_LAYOUT_GENERAL: return ImageLayout::ShaderWrite;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return ImageLayout::RenderTarget;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return ImageLayout::DepthStencilWrite;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return ImageLayout::DepthStencilRead;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ImageLayout::ShaderRead;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return ImageLayout::TransferSource;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return ImageLayout::TransferDestination;
			case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR: return ImageLayout::VideoDecodeWrite;
			case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR: return ImageLayout::VideoDecodeRead;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return ImageLayout::Present;
			default:
			break;
		}

		VT_ENSURE(false);
		return ImageLayout::Undefined;
	}

	const VkImageLayout GetVkImageLayoutFromImageLayout(ImageLayout layout)
	{
		switch (layout)
		{
			case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
			case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			case ImageLayout::RenderTarget: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case ImageLayout::ShaderWrite: return VK_IMAGE_LAYOUT_GENERAL;
			case ImageLayout::DepthStencilWrite: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case ImageLayout::DepthStencilRead: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			case ImageLayout::ShaderRead: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case ImageLayout::TransferSource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case ImageLayout::TransferDestination: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case ImageLayout::VideoDecodeRead: return VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR;
			case ImageLayout::VideoDecodeWrite: return VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;
		}

		VT_ENSURE(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}
