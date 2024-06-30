#include "dxpch.h"
#include "D3D12Image2D.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"
#include "VoltRHI/Images/ImageUtility.h"
#include "VoltRHI/RHIProxy.h"

#include "VoltD3D12/Images/D3D12ImageView.h"

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



	D3D12Image2D::D3D12Image2D(const ImageSpecification& specification, const void* data)
		: m_specification(specification)
	{
		Invalidate(specification.width, specification.height, data);
		SetName(specification.debugName);
	}

	D3D12Image2D::D3D12Image2D(const ImageSpecification& specification, RefPtr<Allocator> customAllocator, const void* data)
		: m_specification(specification), m_customAllocator(customAllocator), m_allocatedUsingCustomAllocator(true)
	{
		Invalidate(specification.width, specification.height, data);
		SetName(specification.debugName);
	}

	D3D12Image2D::D3D12Image2D(const SwapchainImageSpecification& specification)
		: m_swapchainImageData(specification), m_isSwapchainImage(true)
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
		return m_allocation->GetResourceHandle<ID3D12Resource*>();
	}

	Buffer D3D12Image2D::ReadPixelInternal(const uint32_t x, const uint32_t y, const size_t stride)
	{
		return Buffer();
	}

	void D3D12Image2D::InvalidateSwapchainImage(const SwapchainImageSpecification& specification)
	{
	}

	void D3D12Image2D::Invalidate(const uint32_t width, const uint32_t height, const void* data)
	{
		Release();

		m_specification.width = width;
		m_specification.height = height;

		if (m_allocatedUsingCustomAllocator)
		{
			m_allocation = m_customAllocator->CreateImage(m_specification, m_specification.memoryUsage);
		}
		else
		{
			m_allocation = GraphicsContext::GetDefaultAllocator().CreateImage(m_specification, m_specification.memoryUsage);
		}
	}

	void D3D12Image2D::Release()
	{
		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocatedUsingCustomAllocator = m_allocatedUsingCustomAllocator, customAllocator = m_customAllocator, allocation = m_allocation]()
		{
			if (allocatedUsingCustomAllocator)
			{
				customAllocator->DestroyImage(allocation);
			}
			else
			{
				GraphicsContext::GetDefaultAllocator().DestroyImage(allocation);
			}
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
		return ImageLayout();
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
		m_allocation->GetResourceHandle<ID3D12Resource*>()->SetName(wname.c_str());
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
