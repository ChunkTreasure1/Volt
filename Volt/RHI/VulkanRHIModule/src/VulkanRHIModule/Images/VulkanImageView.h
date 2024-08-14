#pragma once

#include "VulkanRHIModule/Core.h"
#include <RHIModule/Images/ImageView.h>
	
struct VkImageView_T;

namespace Volt::RHI
{
	class VulkanImageView : public ImageView
	{
	public:
		VulkanImageView(const ImageViewSpecification& specification);
		~VulkanImageView() override;

		const PixelFormat GetFormat() const;
		const ImageAspect GetImageAspect() const override;
		const uint64_t GetDeviceAddress() const override;
		const ImageUsage GetImageUsage() const override;
		const ImageViewType GetViewType() const override;
		const bool IsSwapchainView() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		ImageViewSpecification m_specification{};

		VkImageView_T* m_imageView = nullptr;

		PixelFormat m_format;
		ImageAspect m_imageAspect;
		ImageUsage m_imageUsage;
		bool m_isSwapchainImage;
	};
}
