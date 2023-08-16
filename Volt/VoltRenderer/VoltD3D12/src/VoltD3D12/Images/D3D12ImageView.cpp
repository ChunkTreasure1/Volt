#include "dxpch.h"
#include "D3D12ImageView.h"

#include "VoltRHI/Images/ImageUtility.h"

#include "VoltD3D12/Images/D3D12Image2D.h"

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

#include "VoltD3D12/Common/D3D12DescriptorHeapManager.h"

namespace Volt::RHI
{

	D3D12ImageView::D3D12ImageView(const ImageViewSpecification specification) : m_specs(specification)
	{
		m_view = new CD3DX12_CPU_DESCRIPTOR_HANDLE;

		auto image = specification.image.lock()->As<D3D12Image2D>();

		auto allocImage = image->GetHandle<AllocatedImage*>();
		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		if (!Utility::IsDepthFormat(image->GetFormat()))
		{
			m_viewID = D3D12DescriptorHeapManager::CreateNewRTVHandle(*m_view);

			device->CreateRenderTargetView(allocImage->texture, nullptr, *m_view);
		}
		else
		{
			m_viewID = D3D12DescriptorHeapManager::CreateNewDSVHandle(*m_view);

			device->CreateDepthStencilView(allocImage->texture, nullptr, *m_view);
		}
	}



	void* D3D12ImageView::GetHandleImpl()
	{
		return m_view;
	}
}
