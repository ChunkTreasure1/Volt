#include "vkpch.h"
#include "VulkanImageView.h"

#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanImageView::VulkanImageView(const ImageViewSpecification& specification)
		: m_specification(specification)
	{
		auto image = specification.image;
		const auto format = image->GetFormat();

		VkImageAspectFlags aspectMask = Utility::IsDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(format))
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = Utility::VoltToVulkanViewType(specification.viewType);
		viewInfo.format = Utility::VoltToVulkanFormat(specification.image->GetFormat());
		viewInfo.flags = 0;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = aspectMask;
		viewInfo.subresourceRange.baseMipLevel = specification.baseMipLevel;
		viewInfo.subresourceRange.baseArrayLayer = specification.baseArrayLayer;
		viewInfo.subresourceRange.levelCount = specification.mipCount;
		viewInfo.subresourceRange.layerCount = specification.layerCount;
		viewInfo.image = image->GetHandle<VkImage>();

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateImageView(device->GetHandle<VkDevice>(), &viewInfo, nullptr, &m_imageView));
	}

	VulkanImageView::~VulkanImageView()
	{
		GraphicsContext::DestroyResource([imageView = m_imageView]()
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroyImageView(device->GetHandle<VkDevice>(), imageView, nullptr);
		});

		m_imageView = nullptr;
	}

	const Format VulkanImageView::GetFormat() const
	{
		return m_specification.image->GetFormat();
	}

	void* VulkanImageView::GetHandleImpl() const
	{
		return m_imageView;
	}
}
