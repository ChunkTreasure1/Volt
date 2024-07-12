#pragma once

#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/Images/Image2D.h>


namespace Volt::RHI
{
	class D3D12Image2D final : public Image2D
	{
	public:
		D3D12Image2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator);
		D3D12Image2D(const SwapchainImageSpecification& specification);

		~D3D12Image2D() override;

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

		const ImageAspect GetImageAspect() const override;

	protected:
		void* GetHandleImpl() const override;
		Buffer ReadPixelInternal(const uint32_t x, const uint32_t y, const size_t stride) override;

	private:
		struct SwapchainImageData
		{
			ID3D12Resource* image = nullptr;
		};

		void InvalidateSwapchainImage(const SwapchainImageSpecification& specification);
		void InitializeWithData(const void* data);

		void TransitionToLayout(ImageLayout targetLayout);

		ImageSpecification m_specification;
		SwapchainImageData m_swapchainImageData;

		RefPtr<Allocation> m_allocation;
		RefPtr<Allocator> m_allocator;

		bool m_isSwapchainImage = false;

		std::map<int32_t, std::map<int32_t, RefPtr<ImageView>>> m_imageViews; // Layer -> Mip -> View
		std::map<int32_t, RefPtr<ImageView>> m_arrayImageViews;
	};
}
