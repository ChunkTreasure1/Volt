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
		using ImageLayoutInt = uint32_t;

		VulkanImage2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator);
		VulkanImage2D(const SwapchainImageSpecification& specification);
		~VulkanImage2D() override;

		void Invalidate(const uint32_t width, const uint32_t height, const void* data) override;
		void Release() override;
		void GenerateMips() override;

		const RefPtr<ImageView> GetView(const int32_t mip, const int32_t layer) override;
		const RefPtr<ImageView> GetArrayView(const int32_t mip /* = -1 */) override;

		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetMipCount() const override;
		const uint32_t GetLayerCount() const override;
		const PixelFormat GetFormat() const override;
		const ImageUsage GetUsage() const override;
		const uint32_t CalculateMipCount() const override;
		const bool IsSwapchainImage() const override;

		inline constexpr ResourceType GetType() const override { return ResourceType::Image2D; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

		inline const ImageAspect GetImageAspect() const override { return m_imageAspect; }

	protected:
		void* GetHandleImpl() const override;
		Buffer ReadPixelInternal(const uint32_t x, const uint32_t y, const size_t stride) override;

	private:
		struct SwapchainImageData
		{
			VkImage_T* image = nullptr;
		};

		void InvalidateSwapchainImage(const SwapchainImageSpecification& specification);
		void TransitionToLayout(ImageLayout targetLayout);
		void InitializeWithData(const void* data);

		ImageSpecification m_specification;
		SwapchainImageData m_swapchainImageData;

		RefPtr<Allocation> m_allocation;
		WeakPtr<Allocator> m_allocator;

		bool m_hasGeneratedMips = false;
		bool m_isSwapchainImage = false;

		ImageAspect m_imageAspect = ImageAspect::None;

		std::map<int32_t, std::map<int32_t, RefPtr<ImageView>>> m_imageViews; // Layer -> Mip -> View
		std::map<int32_t, RefPtr<ImageView>> m_arrayImageViews;
	};
}
