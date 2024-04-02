#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	class ImageView;
	class Allocator;
	class Swapchain;

	struct SwapchainImageSpecification
	{
		Swapchain* swapchain = nullptr;
		uint32_t imageIndex;
	};

	class Image2D : public RHIResource
	{
	public:
		virtual void Invalidate(const uint32_t width, const uint32_t height, const void* data = nullptr) = 0;
		virtual void Release() = 0;
		virtual void GenerateMips() = 0;

		virtual const Ref<ImageView> GetView(const int32_t mip = -1, const int32_t layer = -1) = 0;
		virtual const Ref<ImageView> GetArrayView(const int32_t mip = -1) = 0;

		virtual const ImageAspect GetImageAspect() const = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const PixelFormat GetFormat() const = 0;
		virtual const ImageUsage GetUsage() const = 0;
		virtual const uint32_t CalculateMipCount() const = 0;
		virtual const bool IsSwapchainImage() const = 0;

		static Ref<Image2D> Create(const ImageSpecification& specification, const void* data = nullptr);
		static Ref<Image2D> Create(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data = nullptr);
		static Ref<Image2D> Create(const SwapchainImageSpecification& specification);

	protected:
		Image2D() = default;
		~Image2D() override = default;
	};
}
