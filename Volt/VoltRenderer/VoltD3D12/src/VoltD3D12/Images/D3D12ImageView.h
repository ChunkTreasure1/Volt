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
		void CreateRTVDSV();
		void CreateSRV();
		void CreateUAV();

		ImageViewSpecification m_specification;

		struct ViewData
		{
			size_t id;
			CD3DX12_CPU_DESCRIPTOR_HANDLE view;
			bool valid = false;
		};

		ViewData m_srv;
		ViewData m_rtvDsv;
		ViewData m_uav;

		bool m_hasDSV = false;
	};
}
