#include "dxpch.h"
#include "D3D12ImageView.h"

#include "VoltRHI/Images/ImageUtility.h"

#include "VoltD3D12/Images/D3D12Image2D.h"

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

#include "VoltD3D12/Common/D3D12DescriptorHeapManager.h"

namespace Volt::RHI
{

	D3D12ImageView::D3D12ImageView(const ImageViewSpecification specification) 
		: m_specification(specification)
	{
		auto image = specification.image->As<D3D12Image2D>();
		const auto imageUsage = image->GetUsage();

		if (imageUsage == ImageUsage::Attachment)
		{
			CreateRTVDSV();
		}
		else if (imageUsage == ImageUsage::AttachmentStorage)	
		{
			CreateRTVDSV();
			CreateUAV();
		}
		else if (imageUsage == ImageUsage::Storage)
		{
			CreateUAV();
		}

		CreateSRV();
	}

	D3D12ImageView::~D3D12ImageView()
	{
		// #TODO_Ivar: return stuff
	}

	void* D3D12ImageView::GetHandleImpl() const
	{
		return nullptr;
	}

	const ImageAspect D3D12ImageView::GetImageAspect() const
	{
		return m_specification.image->GetImageAspect();
	}

	const uint64_t D3D12ImageView::GetDeviceAddress() const
	{
		return 0;
	}

	const ImageUsage D3D12ImageView::GetImageUsage() const
	{
		return m_specification.image->GetUsage();
	}

	const ImageViewType D3D12ImageView::GetViewType() const
	{
		return m_specification.viewType;
	}

	const bool D3D12ImageView::IsSwapchainView() const
	{
		return false;
	}

	void D3D12ImageView::CreateRTVDSV()
	{
		const auto format = m_specification.image->GetFormat();
		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		auto resourcePtr = m_specification.image->GetHandle<ID3D12Resource*>();

		if (Utility::IsDepthFormat(format))
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};

			if (m_specification.viewType == ImageViewType::View2D)
			{
				viewDesc.Format = ConvertFormatToD3D12Format(format);
				viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				viewDesc.Flags = !Utility::IsStencilFormat(format) ? D3D12_DSV_FLAG_READ_ONLY_DEPTH : D3D12_DSV_FLAG_NONE;
				viewDesc.Texture2D.MipSlice = m_specification.baseArrayLayer;
			}

			m_rtvDsv.id = D3D12DescriptorHeapManager::CreateNewDSVHandle(m_rtvDsv.view);
			m_rtvDsv.valid = true;
			m_hasDSV = true;

			device->CreateDepthStencilView(resourcePtr, &viewDesc, m_rtvDsv.view);
		}
		else
		{
			D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};

			if (m_specification.viewType == ImageViewType::View2D)
			{
				viewDesc.Format = ConvertFormatToD3D12Format(format);
				viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipSlice = m_specification.baseMipLevel;
				viewDesc.Texture2D.PlaneSlice = m_specification.baseArrayLayer;
			}

			m_rtvDsv.id = D3D12DescriptorHeapManager::CreateNewRTVHandle(m_rtvDsv.view);
			m_rtvDsv.valid = true;

			device->CreateRenderTargetView(resourcePtr, &viewDesc, m_rtvDsv.view);
		}
	}

	void D3D12ImageView::CreateSRV()
	{
		//const auto format = m_specification.image->GetFormat();
		//auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		////auto resourcePtr = m_specification.image->GetHandle<ID3D12Resource*>();

		//D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};

		//if (m_specification.viewType == ImageViewType::View2D)
		//{
		//	viewDesc.Format = ConvertFormatToD3D12Format(format);
		//	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//	viewDesc.Texture2D.MostDetailedMip = m_specification.baseMipLevel;
		//	viewDesc.Texture2D.MipLevels = m_specification.mipCount;
		//	viewDesc.Texture2D.PlaneSlice = m_specification.baseArrayLayer;
		//	viewDesc.Texture2D.ResourceMinLODClamp = 0.f;
		//}
	}

	void D3D12ImageView::CreateUAV()
	{

	}
}
