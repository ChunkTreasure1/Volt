#pragma once

#include <VoltRHI/Images/Image2D.h>

struct VkImage_T;
struct VmaAllocation_T;

namespace Volt::RHI
{
	class VulkanImage2D final : public Image2D
	{
	public:
		typedef uint32_t ImageLayout;
		typedef uint32_t ImageAspect;

		VulkanImage2D(const ImageSpecification& specification, const void* data);
		~VulkanImage2D() override;

		void Invalidate(const uint32_t width, const uint32_t height, const void* data) override;
		void Release() override;
		void GenerateMips() override;

		const Ref<ImageView> GetView(const int32_t mip, const int32_t layer) override;

		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const Format GetFormat() const override;
		const uint32_t CalculateMipCount() const override;

	protected:
		void* GetHandleImpl() override;

	private:
		friend class VulkanCommandBuffer;

		void TransitionToLayout(ImageLayout targetLayout);

		ImageSpecification m_specification;

		VmaAllocation_T* m_allocation = nullptr;
		VkImage_T* m_image = nullptr;

		bool m_hasGeneratedMips = false;

		ImageLayout m_currentImageLayout = 0;
		ImageAspect m_imageAspect = 0;

		std::map<int32_t, std::map<int32_t, Ref<ImageView>>> m_imageViews; // Layer -> Mip -> View
	};
}
