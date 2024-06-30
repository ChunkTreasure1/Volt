#pragma once

#include "VoltRHI/Core/RHIResource.h"
#include "VoltRHI/Memory/Allocator.h"

#include <CoreUtilities/Buffer/Buffer.h>

namespace Volt::RHI
{
	class ImageView;
	class Swapchain;

	class VTRHI_API Image2D : public RHIResource
	{
	public:
		virtual void Invalidate(const uint32_t width, const uint32_t height, const void* data = nullptr) = 0;
		virtual void Release() = 0;
		virtual void GenerateMips() = 0;

		virtual const RefPtr<ImageView> GetView(const int32_t mip = -1, const int32_t layer = -1) = 0;
		virtual const RefPtr<ImageView> GetArrayView(const int32_t mip = -1) = 0;

		virtual const ImageAspect GetImageAspect() const = 0;
		virtual const ImageLayout GetImageLayout() const = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetMipCount() const = 0;
		virtual const PixelFormat GetFormat() const = 0;
		virtual const ImageUsage GetUsage() const = 0;
		virtual const uint32_t CalculateMipCount() const = 0;
		virtual const bool IsSwapchainImage() const = 0;

		template<typename T>
		VT_INLINE T ReadPixel(uint32_t x, uint32_t y);

		static RefPtr<Image2D> Create(const ImageSpecification& specification, const void* data = nullptr, RefPtr<Allocator> customAllocator = nullptr);
		static RefPtr<Image2D> Create(const SwapchainImageSpecification& specification);

	protected:
		virtual Buffer ReadPixelInternal(const uint32_t x, const uint32_t y, const size_t stride) = 0;

		Image2D() = default;
		~Image2D() override = default;
	};

	template<typename T>
	VT_INLINE T Image2D::ReadPixel(uint32_t x, uint32_t y)
	{
		auto buffer = ReadPixelInternal(x, y, sizeof(T));
		T value = *buffer.As<T>();
		buffer.Release();

		return value;
	}
}
