#pragma once

#include <VoltRHI/Images/Image3D.h>

#include <CoreUtilities/Containers/Map.h>

namespace Volt::RHI
{
	class VulkanImage3D final : public Image3D
	{
	public:
		VulkanImage3D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator);
		~VulkanImage3D();

		void Invalidate(const uint32_t width, const uint32_t height, const uint32_t depth, const void* data /* = nullptr */) override;
		void Release() override;
		void GenerateMips() override;

		const RefPtr<ImageView> GetView(const int32_t mip /* = -1 */, const int32_t layer /* = -1 */) override;
		const RefPtr<ImageView> GetArrayView(const int32_t mip /* = -1 */) override;

		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetDepth() const override;
		const uint32_t GetMipCount() const override;
		const uint32_t GetLayerCount() const override;
		const PixelFormat GetFormat() const override;
		const ImageUsage GetUsage() const override;
		const uint32_t CalculateMipCount() const override;

		inline constexpr ResourceType GetType() const override { return ResourceType::Image3D; }
		void SetName(std::string_view name) override;
		const uint64_t GetDeviceAddress() const override;
		const uint64_t GetByteSize() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void TransitionToLayout(ImageLayout targetLayout);

		ImageSpecification m_specification;

		RefPtr<Allocation> m_allocation;
		WeakPtr<Allocator> m_allocator;

		bool m_hasGeneratedMips = false;

		vt::map<int32_t, vt::map<int32_t, RefPtr<ImageView>>> m_imageViews; // Layer -> Mip -> View
		vt::map<int32_t, RefPtr<ImageView>> m_arrayImageViews;
	};
}
