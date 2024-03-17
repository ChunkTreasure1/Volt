#pragma once

#include <VoltRHI/Images/ImageView.h>
	
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

	protected:
		void* GetHandleImpl() const override;

	private:
		ImageViewSpecification m_specification{};

		VkImageView_T* m_imageView = nullptr;
	};
}
