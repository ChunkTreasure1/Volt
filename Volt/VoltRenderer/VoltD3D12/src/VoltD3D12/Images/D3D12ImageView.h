#pragma once
#include "VoltRHI/Images/ImageView.h"

struct CD3DX12_CPU_DESCRIPTOR_HANDLE;

namespace Volt::RHI
{
	class D3D12ImageView final : public ImageView
	{
	public:
		D3D12ImageView(const ImageViewSpecification specification);
		~D3D12ImageView() override;
		void* GetHandleImpl() const override;

		const ImageAspect GetImageAspect() const override;
		const uint64_t GetDeviceAddress() const override;
		const ImageUsage GetImageUsage() const override;
		const ImageViewType GetViewType() const override;
		const bool IsSwapchainView() const override;

	private:
		ImageViewSpecification m_specs;
		size_t m_viewID;
		CD3DX12_CPU_DESCRIPTOR_HANDLE* m_view = {};
	};
}
