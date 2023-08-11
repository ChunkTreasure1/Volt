#pragma once

#include "VoltRHI/Core/RHIResource.h"

namespace Volt::RHI
{
	struct ImageSpecification
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 0;
		uint32_t layers = 1;
		uint32_t mips = 1;

		Format format = Format::R8G8B8A8_UNORM;
		ImageUsage usage = ImageUsage::Texture;
		TextureWrap wrap = TextureWrap::Repeat;
		TextureFilter filter = TextureFilter::Linear;

		MemoryUsage memoryUsage = MemoryUsage::GPUOnly;

		AnisotopyLevel anisoLevel = AnisotopyLevel::None;
		std::string debugName;

		bool isCubeMap = false;
		bool generateMips = false;
	};

	class ImageView;
	class Image2D : public RHIResource
	{
	public:
		virtual void Invalidate(const uint32_t width, const uint32_t height, const void* data = nullptr) = 0;
		virtual void Release() = 0;
		virtual void GenerateMips() = 0;

		virtual const Ref<ImageView> GetView(const int32_t mip = -1, const int32_t layer = -1) = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const Format GetFormat() const = 0;
		virtual const uint32_t CalculateMipCount() const = 0;

		static Ref<Image2D> Create(const ImageSpecification& specification, const void* data = nullptr);

	protected:
		Image2D() = default;
		~Image2D() override = default;
	};
}
