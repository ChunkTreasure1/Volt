#pragma once

#include <VoltRHI/Images/Image2D.h>

struct VkImage_T;
struct VmaAllocation_T;

namespace Volt::RHI
{
	class Allocation;
	class VulkanImage2D final : public Image2D
	{
	public:
		using ImageLayout = uint32_t;
		using ImageAspect = uint32_t;

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

		inline constexpr ResourceType GetType() const override { return ResourceType::Image2D; }
		void SetName(std::string_view name) override;

		const ImageLayout GetCurrentLayout() const { return m_currentImageLayout; }
		const ImageAspect GetImageAspect() const { return m_imageAspect; }

	protected:
		void* GetHandleImpl() override;

	private:
		friend class VulkanCommandBuffer;

		void TransitionToLayout(ImageLayout targetLayout);

		ImageSpecification m_specification;

		Ref<Allocation> m_allocation;

		bool m_hasGeneratedMips = false;

		ImageLayout m_currentImageLayout = 0;
		ImageAspect m_imageAspect = 0;

		std::map<int32_t, std::map<int32_t, Ref<ImageView>>> m_imageViews; // Layer -> Mip -> View
	};
}
