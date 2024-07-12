#include "dxpch.h"
#include "D3D12ImageView.h"

#include "VoltRHI/Images/ImageUtility.h"
#include "VoltD3D12/Images/D3D12Image2D.h"

#include "VoltD3D12/Descriptors/DescriptorUtility.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Common/D3D12Helpers.h"

namespace Volt::RHI
{

	D3D12ImageView::D3D12ImageView(const ImageViewSpecification specification) 
		: m_specification(specification)
	{
		auto image = specification.image->As<D3D12Image2D>();
		const auto imageUsage = image->GetUsage();

		m_viewUsage = D3D12ViewType::None;

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
		if (m_rtvDsvDescriptor.IsValid())
		{
			DescriptorUtility::FreeDescriptorPointer(m_rtvDsvDescriptor);
		}

		if (m_srvDescriptor.IsValid())
		{
			DescriptorUtility::FreeDescriptorPointer(m_srvDescriptor);
		}

		if (m_uavDescriptor.IsValid())
		{
			DescriptorUtility::FreeDescriptorPointer(m_uavDescriptor);
		}
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
		auto* devicePtr = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		auto* resourcePtr = m_specification.image->GetHandle<ID3D12Resource*>();

		if (Utility::IsDepthFormat(format))
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
			const auto d3d12Format = ConvertFormatToD3D12Format(format);
			const bool isTypeless = Utility::IsFormatTypeless(d3d12Format);

			if (isTypeless)
			{
				viewDesc.Format = Utility::GetDSVFormatFromTypeless(d3d12Format);
			}
			else
			{
				viewDesc.Format = d3d12Format;
			}


			if (m_specification.viewType == ImageViewType::View2D)
			{
				viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipSlice = m_specification.baseMipLevel;
			}
			else if (m_specification.viewType == ImageViewType::View2DArray)
			{
				viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				viewDesc.Texture2DArray.ArraySize = m_specification.layerCount;
				viewDesc.Texture2DArray.FirstArraySlice = m_specification.baseArrayLayer;
				viewDesc.Texture2DArray.MipSlice = m_specification.baseMipLevel;
			}

			m_viewUsage = D3D12ViewType::DSV;
			m_rtvDsvDescriptor = DescriptorUtility::AllocateDescriptorPointer(D3D12DescriptorType::DSV);

			devicePtr->CreateDepthStencilView(resourcePtr, &viewDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_rtvDsvDescriptor.GetCPUPointer()));
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

			m_viewUsage = D3D12ViewType::RTV;
			m_rtvDsvDescriptor = DescriptorUtility::AllocateDescriptorPointer(D3D12DescriptorType::RTV);
		
			devicePtr->CreateRenderTargetView(resourcePtr, &viewDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_rtvDsvDescriptor.GetCPUPointer()));
		}
	}

	void D3D12ImageView::CreateSRV()
	{
		const auto format = m_specification.image->GetFormat();
		auto* devicePtr = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		auto* resourcePtr = m_specification.image->GetHandle<ID3D12Resource*>();

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		const auto d3d12Format = ConvertFormatToD3D12Format(format);
		const bool isTypeless = Utility::IsFormatTypeless(d3d12Format);

		if (isTypeless)
		{
			viewDesc.Format = Utility::GetSRVUAVFormatFromTypeless(d3d12Format);
		}
		else
		{
			viewDesc.Format = d3d12Format;
		}

		if (m_specification.viewType == ImageViewType::View2D)
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MostDetailedMip = m_specification.baseMipLevel;
			viewDesc.Texture2D.MipLevels = m_specification.mipCount;
			viewDesc.Texture2D.PlaneSlice = 0;
			viewDesc.Texture2D.ResourceMinLODClamp = 0.f;
		}
		else if (m_specification.viewType == ImageViewType::View2DArray)
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MostDetailedMip = m_specification.baseMipLevel;
			viewDesc.Texture2DArray.MipLevels = m_specification.mipCount;
			viewDesc.Texture2DArray.ArraySize = m_specification.layerCount;
			viewDesc.Texture2DArray.FirstArraySlice = m_specification.baseArrayLayer;
			viewDesc.Texture2DArray.ResourceMinLODClamp = 0.f;
			viewDesc.Texture2DArray.PlaneSlice = 0;
		}
		else if (m_specification.viewType == ImageViewType::ViewCube)
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.TextureCube.MipLevels = m_specification.mipCount;
			viewDesc.TextureCube.MostDetailedMip = m_specification.baseMipLevel;
			viewDesc.TextureCube.ResourceMinLODClamp = 0.f;
		}

		m_viewUsage |= D3D12ViewType::SRV;
		m_srvDescriptor = DescriptorUtility::AllocateDescriptorPointer(D3D12DescriptorType::CBV_SRV_UAV);
		devicePtr->CreateShaderResourceView(resourcePtr, &viewDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_srvDescriptor.GetCPUPointer()));
	}

	void D3D12ImageView::CreateUAV()
	{
		if (m_specification.viewType == ImageViewType::ViewCube)
		{
			return;
		}

		const auto format = m_specification.image->GetFormat();
		auto* devicePtr = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		auto* resourcePtr = m_specification.image->GetHandle<ID3D12Resource*>();

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc{};

		const auto d3d12Format = ConvertFormatToD3D12Format(format);
		const bool isTypeless = Utility::IsFormatTypeless(d3d12Format);

		if (isTypeless)
		{
			viewDesc.Format = Utility::GetSRVUAVFormatFromTypeless(d3d12Format);
		}
		else
		{
			viewDesc.Format = d3d12Format;
		}

		if (m_specification.viewType == ImageViewType::View2D)
		{
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = m_specification.baseMipLevel;
			viewDesc.Texture2D.PlaneSlice = m_specification.baseArrayLayer;
		}
		else if (m_specification.viewType == ImageViewType::View2DArray)
		{
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.ArraySize = m_specification.layerCount;
			viewDesc.Texture2DArray.FirstArraySlice = m_specification.baseArrayLayer;
			viewDesc.Texture2DArray.MipSlice = m_specification.baseMipLevel;
			viewDesc.Texture2DArray.PlaneSlice = 0;
		}

		m_viewUsage |= D3D12ViewType::UAV;
		m_uavDescriptor = DescriptorUtility::AllocateDescriptorPointer(D3D12DescriptorType::CBV_SRV_UAV);
		devicePtr->CreateUnorderedAccessView(resourcePtr, nullptr, &viewDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_uavDescriptor.GetCPUPointer()));
	}
}
