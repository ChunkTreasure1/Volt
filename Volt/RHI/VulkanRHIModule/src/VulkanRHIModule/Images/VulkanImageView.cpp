#include "vkpch.h"
#include "VulkanImageView.h"

#include "VulkanRHIModule/Common/VulkanHelpers.h"
#include "VulkanRHIModule/Common/VulkanCommon.h"

#include <RHIModule/Images/Image2D.h>
#include <RHIModule/Images/Image3D.h>
#include <RHIModule/Images/ImageUtility.h>
#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>

#include <RHIModule/RHIProxy.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanImageView::VulkanImageView(const ImageViewSpecification& specification)
		: m_specification(specification)
	{
		auto image = specification.image;
		if (image->GetType() == ResourceType::Image2D)
		{
			auto image2D = image->As<Image2D>();
			m_format = image2D->GetFormat();
			m_imageUsage = image2D->GetUsage();
			m_imageAspect = image2D->GetImageAspect();
			m_isSwapchainImage = image2D->IsSwapchainImage();
		}
		else if (image->GetType() == ResourceType::Image3D)
		{
			auto image3D = image->As<Image3D>();
			m_format = image3D->GetFormat();
			m_imageUsage = image3D->GetUsage();
			m_imageAspect = ImageAspect::Color;
			m_isSwapchainImage = false;
		}

		VkImageAspectFlags aspectMask = Utility::IsDepthFormat(m_format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_format))
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = Utility::VoltToVulkanViewType(specification.viewType);
		viewInfo.format = Utility::VoltToVulkanFormat(m_format);
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
		RHIProxy::GetInstance().DestroyResource([imageView = m_imageView]()
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroyImageView(device->GetHandle<VkDevice>(), imageView, nullptr);
		});

		m_imageView = nullptr;
	}

	const PixelFormat VulkanImageView::GetFormat() const
	{
		return m_format;
	}

	const ImageAspect VulkanImageView::GetImageAspect() const
	{
		return m_imageAspect;
	}

	const uint64_t VulkanImageView::GetDeviceAddress() const
	{
		return m_specification.image->GetDeviceAddress();
	}

	const ImageUsage VulkanImageView::GetImageUsage() const
	{
		return m_imageUsage;
	}

	const ImageViewType VulkanImageView::GetViewType() const
	{
		return m_specification.viewType;
	}

	const bool VulkanImageView::IsSwapchainView() const
	{
		return m_isSwapchainImage;
	}

	void* VulkanImageView::GetHandleImpl() const
	{
		return m_imageView;
	}
}
