#pragma once

#include "RHIModule/Core/RHIResource.h"
#include "RHIModule/Memory/Allocator.h"

namespace Volt::RHI
{
	class ImageView;

	class VTRHI_API Image3D : public RHIResource
	{
	public:
		virtual void Invalidate(const uint32_t width, const uint32_t height, const uint32_t depth, const void* data = nullptr) = 0;
		virtual void Release() = 0;
		virtual void GenerateMips() = 0;

		virtual const RefPtr<ImageView> GetView(const int32_t mip = -1, const int32_t layer = -1) = 0;
		virtual const RefPtr<ImageView> GetArrayView(const int32_t mip = -1) = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetDepth() const = 0;
		virtual const uint32_t GetMipCount() const = 0;
		virtual const uint32_t GetLayerCount() const = 0;
		virtual const PixelFormat GetFormat() const = 0;
		virtual const ImageUsage GetUsage() const = 0;
		virtual const uint32_t CalculateMipCount() const = 0;

		static RefPtr<Image3D> Create(const ImageSpecification& specification, const void* data = nullptr, RefPtr<Allocator> allocator = nullptr);
	
	protected:
		Image3D() = default;
		~Image3D() override = default;
	};
}
