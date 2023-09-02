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

		const Format GetFormat() const;

	protected:
		void* GetHandleImpl() const override;

	private:
		ImageViewSpecification m_specification{};

		VkImageView_T* m_imageView = nullptr;
	};
}
