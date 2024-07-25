#pragma once
#include "VoltD3D12/Common/D3D12Common.h"
#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Images/ImageView.h>

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

		VT_NODISCARD VT_INLINE const D3D12DescriptorPointer& GetRTVDSVDescriptor() const { return m_rtvDsvDescriptor; }
		VT_NODISCARD VT_INLINE const D3D12DescriptorPointer& GetSRVDescriptor() const { return m_srvDescriptor; }
		VT_NODISCARD VT_INLINE const D3D12DescriptorPointer& GetUAVDescriptor() const { return m_uavDescriptor; }
		VT_NODISCARD VT_INLINE D3D12ViewType GetD3D12ViewType() const { return m_viewUsage; }

	private:
		void CreateRTVDSV();
		void CreateSRV();
		void CreateUAV();

		ImageViewSpecification m_specification;

		D3D12ViewType m_viewUsage;
		D3D12DescriptorPointer m_rtvDsvDescriptor;
		D3D12DescriptorPointer m_srvDescriptor;
		D3D12DescriptorPointer m_uavDescriptor;

		PixelFormat m_format;
		ImageAspect m_imageAspect;
		ImageUsage m_imageUsage;
		bool m_isSwapchainImage;
	};
}
