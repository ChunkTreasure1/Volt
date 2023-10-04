#pragma once
#include <VoltRHI/Images/Image2D.h>

#include "VoltD3D12/Common/D3D12AllocatorForward.h"

namespace Volt::RHI
{
	class D3D12Image2D final : public Image2D
	{
	public:
		D3D12Image2D(const ImageSpecification& specification, const void* data);
		D3D12Image2D(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data);

		~D3D12Image2D() override;

		void* GetHandleImpl() const override;
		void Invalidate(const uint32_t width, const uint32_t height, const void* data) override;
		void Release() override;
		void GenerateMips() override;
		const Ref<ImageView> GetView(const int32_t mip, const int32_t layer) override;
		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const PixelFormat GetFormat() const override;
		const ImageAspect GetImageAspect() const override;
		const uint32_t CalculateMipCount() const override;

		inline constexpr ResourceType GetType() const override { return ResourceType::Image2D; }
		void SetName(std::string_view name) override;

	private:
		Ref<ImageView> m_view;
		ImageSpecification m_specs;
		AllocatedImage m_image;
	};
}
