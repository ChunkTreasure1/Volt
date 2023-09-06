#pragma once
#include "VoltRHI/Images/ImageView.h"

struct CD3DX12_CPU_DESCRIPTOR_HANDLE;

namespace Volt::RHI
{
	class D3D12ImageView final : public ImageView
	{
	public:
		D3D12ImageView(const ImageViewSpecification specification);
		void* GetHandleImpl() const override;

		const ImageAspect GetImageAspect() const override;

	private:
		ImageViewSpecification m_specs;
		size_t m_viewID;
		CD3DX12_CPU_DESCRIPTOR_HANDLE* m_view = {};
	};
}
