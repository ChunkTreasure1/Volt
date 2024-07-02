#include "dxpch.h"
#include "D3D12Image2D.h"

#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Graphics/D3D12Swapchain.h"

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	 D3D12_RESOURCE_FLAGS GetFlagFromUsage(ImageUsage usage)
	{
		switch (usage)
		{
			case Volt::RHI::ImageUsage::None: return D3D12_RESOURCE_FLAG_NONE;

			case Volt::RHI::ImageUsage::Texture:
				break;
				
			case Volt::RHI::ImageUsage::Attachment: return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			case Volt::RHI::ImageUsage::AttachmentStorage:
				break;

			case Volt::RHI::ImageUsage::Storage:
				break;
		}
		return D3D12_RESOURCE_FLAG_NONE;
	}

	D3D12Image2D::D3D12Image2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
		: m_specification(specification), m_allocator(allocator)
	{
		if (!allocator)
		{
			m_allocator = GraphicsContext::GetDefaultAllocator();
		}

		Invalidate(specification.width, specification.height, data);
		SetName(specification.debugName);
	}

	D3D12Image2D::D3D12Image2D(const SwapchainImageSpecification& specification)
		: m_isSwapchainImage(true)
	{
		InvalidateSwapchainImage(specification);
		SetName("Swapchain Image");
	}

	D3D12Image2D::~D3D12Image2D()
	{
		Release();
	}

	void* D3D12Image2D::GetHandleImpl() const 
	{
		if (m_isSwapchainImage)
		{
			return m_swapchainImageData.image;
		}
		else
		{
			return m_allocation->GetResourceHandle<ID3D12Resource*>();
		}
	}

	Buffer D3D12Image2D::ReadPixelInternal(const uint32_t x, const uint32_t y, const size_t stride)
	{
		return Buffer();
	}

	void D3D12Image2D::InvalidateSwapchainImage(const SwapchainImageSpecification& specification)
	{
		const auto& d3d12Swapchain = specification.swapchain->AsRef<D3D12Swapchain>();
	
		m_layout = ImageLayout::Undefined;
		m_specification.width = d3d12Swapchain.GetWidth();
		m_specification.height = d3d12Swapchain.GetHeight();
		m_specification.format = d3d12Swapchain.GetFormat();
		m_specification.usage = ImageUsage::Attachment;

		m_swapchainImageData.image = d3d12Swapchain.GetImageAtIndex(specification.imageIndex).Get();
	}

	void D3D12Image2D::Invalidate(const uint32_t width, const uint32_t height, const void* data)
	{
		Release();

		m_specification.width = width;
		m_specification.height = height;
		m_allocation = m_allocator->CreateImage(m_specification, m_specification.memoryUsage);

		if (m_specification.initializeImage)
		{
			// This should match GetResourceStateFromUsage in D3D12Helpers.h
			//switch (m_specification.usage)
			//{
			//	case ImageUsage::Attachment:
			//	case ImageUsage::AttachmentStorage:
			//	{
			//		if ((GetImageAspect() & ImageAspect::Depth) != ImageAspect::None)
			//		{
			//			m_layout = ImageLayout::DepthStencilWrite;
			//		}
			//		else
			//		{
			//			m_layout = ImageLayout::RenderTarget;
			//		}
			//		break;
			//	}

			//	case ImageUsage::Texture:
			//	{
			//		m_layout = ImageLayout::ShaderRead;
			//		break;
			//	}

			//	case ImageUsage::Storage:
			//	{
			//		m_layout = ImageLayout::ShaderWrite;
			//		break;
			//	}
			//}

			m_layout = ImageLayout::Undefined;
		}
	}

	void D3D12Image2D::Release()
	{
		m_imageViews.clear();

		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = m_allocation]()
		{
			allocator->DestroyImage(allocation);
		});

		m_allocation = nullptr;
	}

	void D3D12Image2D::GenerateMips()
	{
	}

	const RefPtr<ImageView> D3D12Image2D::GetView(const int32_t mip, const int32_t layer)
	{
		if (m_imageViews.contains(layer))
		{
			if (m_imageViews.at(layer).contains(mip))
			{
				return m_imageViews.at(layer).at(mip);
			}
		}

		ImageViewSpecification spec{};
		spec.baseArrayLayer = (layer == -1) ? 0 : layer;
		spec.baseMipLevel = (mip == -1) ? 0 : mip;
		spec.layerCount = (layer == -1) ? m_specification.layers : 1;
		spec.mipCount = (mip == -1) ? m_specification.mips : 1;
		spec.viewType = ImageViewType::View2D;

		if (m_specification.isCubeMap)
		{
			spec.layerCount = 6;

			// When using cube array, layer specifies which cubemap index
			if (m_specification.layers > 6 && layer != -1)
			{
				spec.baseArrayLayer = layer * 6u;
			}

			spec.viewType = ImageViewType::ViewCube;
		}
		else if (m_specification.layers > 1)
		{
			spec.viewType = ImageViewType::View2DArray;
		}

		if (m_specification.isCubeMap && m_specification.layers > 6 && layer == -1)
		{
			spec.viewType = ImageViewType::ViewCubeArray;

			spec.layerCount = m_specification.layers;
		}

		spec.image = this;

		RefPtr<ImageView> view = RefPtr<D3D12ImageView>::Create(spec);
		m_imageViews[layer][mip] = view;

		return view;
	}

	const RefPtr<ImageView> D3D12Image2D::GetArrayView(const int32_t mip)
	{
		return RefPtr<ImageView>();
	}

	const uint32_t D3D12Image2D::GetWidth() const
	{
		return m_specification.width;
	}

	const uint32_t D3D12Image2D::GetHeight() const
	{
		return m_specification.height;
	}

	const uint32_t D3D12Image2D::GetMipCount() const
	{
		return m_specification.mips;
	}

	const PixelFormat D3D12Image2D::GetFormat() const
	{
		return m_specification.format;
	}

	const ImageUsage D3D12Image2D::GetUsage() const
	{
		return m_specification.usage;
	}

	const ImageAspect D3D12Image2D::GetImageAspect() const
	{
		return Utility::IsDepthFormat(m_specification.format) ? ImageAspect::Depth : ImageAspect::Color;
	}

	const ImageLayout D3D12Image2D::GetImageLayout() const
	{
		return m_layout;
	}

	const uint32_t D3D12Image2D::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(GetWidth(), GetHeight());
	}

	const bool D3D12Image2D::IsSwapchainImage() const
	{
		return m_isSwapchainImage;
	}

	void D3D12Image2D::SetName(std::string_view name)
	{
		std::wstring wname(name.begin(), name.end());
		if (m_isSwapchainImage)
		{
			m_swapchainImageData.image->SetName(wname.c_str());
		}
		else
		{
			m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(wname.c_str());
		}
	}

	const uint64_t D3D12Image2D::GetDeviceAddress() const
	{
		return 0;
	}

	const uint64_t D3D12Image2D::GetByteSize() const
	{
		return m_allocation->GetSize();
	}
}
