#pragma once

#include "VoltVulkan/Core.h"
#include <VoltRHI/Images/Image2D.h>

struct VkImage_T;

namespace Volt::RHI
{
	class Allocation;
	class Allocator;

	class VulkanImage2D final : public Image2D
	{
	public:
		using ImageLayout = uint32_t;

		VulkanImage2D(const ImageSpecification& specification, const void* data);
		VulkanImage2D(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data);
		VulkanImage2D(const SwapchainImageSpecification& specification);
		~VulkanImage2D() override;

		void Invalidate(const uint32_t width, const uint32_t height, const void* data) override;
		void Release() override;
		void GenerateMips() override;

		const Ref<ImageView> GetView(const int32_t mip, const int32_t layer) override;
		const Ref<ImageView> GetArrayView(const int32_t mip /* = -1 */) override;

		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetMipCount() const override;
		const PixelFormat GetFormat() const override;
		const ImageUsage GetUsage() const override;
		const uint32_t CalculateMipCount() const override;
		const bool IsSwapchainImage() const override;

		inline constexpr ResourceType GetType() const override { return ResourceType::Image2D; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

		const ImageLayout GetCurrentLayout() const { return m_currentImageLayout; }
		void SetCurrentLayout(ImageLayout layout) { m_currentImageLayout = layout; }
		inline const ImageAspect GetImageAspect() const override { return m_imageAspect; }

		void InitializeWithData(const void* data);

	protected:
		void* GetHandleImpl() const override;

	private:
		struct SwapchainImageData
		{
			VkImage_T* image = nullptr;
		};

		void InvalidateSwapchainImage(const SwapchainImageSpecification& specification);
		void TransitionToLayout(ImageLayout targetLayout);

		ImageSpecification m_specification;
		SwapchainImageData m_swapchainImageData;

		Ref<Allocation> m_allocation;
		Weak<Allocator> m_customAllocator;

		bool m_hasGeneratedMips = false;
		bool m_allocatedUsingCustomAllocator = false;
		bool m_isSwapchainImage = false;

		ImageLayout m_currentImageLayout = 0;
		ImageAspect m_imageAspect = ImageAspect::None;

		std::map<int32_t, std::map<int32_t, Ref<ImageView>>> m_imageViews; // Layer -> Mip -> View
		std::map<int32_t, Ref<ImageView>> m_arrayImageViews;
	};
}
