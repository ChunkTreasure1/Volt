#pragma once

#include "RHIModule/Core/RHIResource.h"
#include "RHIModule/Memory/Allocator.h"

#include <CoreUtilities/Buffer/Buffer.h>

namespace Volt::RHI
{
	class ImageView;
	class Swapchain;

	class VTRHI_API Image : public RHIResource
	{
	public:
		virtual void Invalidate(const uint32_t width, const uint32_t height, const uint32_t depth = 0, const void* data = nullptr) = 0;
		virtual void Release() = 0;
		virtual void GenerateMips() = 0;

		virtual RefPtr<ImageView> GetView(const int32_t mip = -1, const int32_t layer = -1) = 0;
		virtual RefPtr<ImageView> GetArrayView(const int32_t mip = -1) = 0;

		virtual const ImageAspect GetImageAspect() const = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetDepth() const = 0;

		virtual const uint32_t GetMipCount() const = 0;
		virtual const uint32_t GetLayerCount() const = 0;
		virtual const PixelFormat GetFormat() const = 0;
		virtual const ImageUsage GetUsage() const = 0;
		virtual const uint32_t CalculateMipCount() const = 0;
		virtual const bool IsSwapchainImage() const = 0;

		template<typename T>
		VT_INLINE T ReadPixel(uint32_t x, uint32_t y, uint32_t z);

		static RefPtr<Image> Create(const ImageSpecification& specification, const void* data = nullptr, RefPtr<Allocator> allocator = nullptr);
		static RefPtr<Image> Create(const SwapchainImageSpecification& specification);

	protected:
		virtual Buffer ReadPixelInternal(const uint32_t x, const uint32_t y, const uint32_t z, const size_t stride) = 0;

		Image() = default;
		~Image() override = default;
	};

	template<typename T>
	VT_INLINE T Image::ReadPixel(uint32_t x, uint32_t y, uint32_t z)
	{
		auto buffer = ReadPixelInternal(x, y, z, sizeof(T));
		T value = *buffer.As<T>();
		buffer.Release();

		return value;
	}
}
