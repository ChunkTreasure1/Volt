#include "dxpch.h"
#include "D3D12RHIModule/Images/D3D12Image.h"

#include "D3D12RHIModule/Images/D3D12ImageView.h"
#include "D3D12RHIModule/Graphics/D3D12Swapchain.h"

#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Images/ImageUtility.h>
#include <RHIModule/RHIProxy.h>

#include <RHIModule/Utility/ResourceUtility.h>

namespace Volt::RHI
{
	D3D12Image::D3D12Image(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
		: m_specification(specification), m_allocator(allocator)
	{
		if (!allocator)
		{
			m_allocator = GraphicsContext::GetDefaultAllocator();
		}

		Invalidate(specification.width, specification.height, m_specification.depth, data);
		SetName(specification.debugName);
	}

	D3D12Image::D3D12Image(const SwapchainImageSpecification& specification)
		: m_isSwapchainImage(true)
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None, ImageLayout::Undefined);

		InvalidateSwapchainImage(specification);
		SetName("Swapchain Image");
	}

	D3D12Image::~D3D12Image()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);
		Release();
	}

	void* D3D12Image::GetHandleImpl() const
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

	Buffer D3D12Image::ReadPixelInternal(const uint32_t x, const uint32_t y, const uint32_t z, const size_t stride)
	{
		return Buffer();
	}

	void D3D12Image::InvalidateSwapchainImage(const SwapchainImageSpecification& specification)
	{
		const auto& d3d12Swapchain = specification.swapchain->AsRef<D3D12Swapchain>();

		m_specification.width = d3d12Swapchain.GetWidth();
		m_specification.height = d3d12Swapchain.GetHeight();
		m_specification.format = d3d12Swapchain.GetFormat();
		m_specification.usage = ImageUsage::Attachment;

		m_swapchainImageData.image = d3d12Swapchain.GetImageAtIndex(specification.imageIndex).Get();

		TransitionToLayout(ImageLayout::RenderTarget);
	}

	void D3D12Image::InitializeWithData(const void* data)
	{
		RefPtr<CommandBuffer> cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Begin();

		ImageCopyData copyData{};
		auto& subResource = copyData.copySubData.emplace_back();
		subResource.data = data;
		subResource.rowPitch = m_specification.width * Utility::GetByteSizePerPixelFromFormat(m_specification.format);
		subResource.slicePitch = m_specification.width * m_specification.height * Utility::GetByteSizePerPixelFromFormat(m_specification.format);

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), this);

			barrier.imageBarrier().dstStage = RHI::BarrierStage::Copy;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::CopyDest;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::CopyDest;
			barrier.imageBarrier().resource = this;

			cmdBuffer->ResourceBarrier({ barrier });
		}

		cmdBuffer->UploadTextureData(this, copyData);

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), this);

			barrier.imageBarrier().dstStage = RHI::BarrierStage::PixelShader | RHI::BarrierStage::ComputeShader;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
			barrier.imageBarrier().resource = this;

			cmdBuffer->ResourceBarrier({ barrier });
		}

		cmdBuffer->End();
		cmdBuffer->Execute();
	}

	void D3D12Image::TransitionToLayout(ImageLayout targetLayout)
	{
		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();
		commandBuffer->Begin();

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), this);

			barrier.imageBarrier().srcAccess = BarrierAccess::None;
			barrier.imageBarrier().dstAccess = BarrierAccess::None;

			barrier.imageBarrier().srcStage = BarrierStage::None;
			barrier.imageBarrier().dstStage = BarrierStage::None;

			barrier.imageBarrier().dstLayout = targetLayout;
			barrier.imageBarrier().resource = this;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->End();
		commandBuffer->Execute();
	}

	void D3D12Image::Invalidate(const uint32_t width, const uint32_t height, const uint32_t depth, const void* data)
	{
		Release();

		m_specification.width = width;
		m_specification.height = height;
		m_specification.depth = depth;
		m_allocation = m_allocator->CreateImage(m_specification, m_specification.memoryUsage);

		ImageLayout targetLayout = ImageLayout::Undefined;

		if (m_specification.imageType == ResourceType::Image3D)
		{
			VT_ENSURE_MSG(m_specification.usage != ImageUsage::Attachment && m_specification.usage != ImageUsage::AttachmentStorage, "Attachment types are not supported for 3D images!");
		}

		switch (m_specification.usage)
		{
			case ImageUsage::Attachment:
			case ImageUsage::AttachmentStorage:
			{
				if ((GetImageAspect() & ImageAspect::Depth) != ImageAspect::None)
				{
					targetLayout = ImageLayout::DepthStencilWrite;
				}
				else
				{
					targetLayout = ImageLayout::RenderTarget;
				}
				break;
			}

			case ImageUsage::Texture:
			{
				targetLayout = ImageLayout::ShaderRead;
				break;
			}

			case ImageUsage::Storage:
			{
				targetLayout = ImageLayout::ShaderWrite;
				break;
			}
		}

		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None, targetLayout);

		if (data)
		{
			InitializeWithData(data);
		}

		if (m_specification.initializeImage)
		{
			TransitionToLayout(targetLayout);
		}

		if (m_specification.generateMips && m_specification.mips > 1)
		{
			GenerateMips();
		}
	}

	void D3D12Image::Release()
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

	void D3D12Image::GenerateMips()
	{
	}

	RefPtr<ImageView> D3D12Image::GetView(const int32_t mip, const int32_t layer)
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

		if (m_specification.imageType == ResourceType::Image1D)
		{
			spec.viewType = ImageViewType::View1D;
		}
		else if (m_specification.imageType == ResourceType::Image2D)
		{
			spec.viewType = ImageViewType::View2D;
		}
		else if (m_specification.imageType == ResourceType::Image3D)
		{
			spec.viewType = ImageViewType::View3D;
		}

		if (m_specification.isCubeMap && m_specification.imageType == ResourceType::Image2D)
		{
			spec.layerCount = 6;

			// When using cube array, layer specifies which cubemap index
			if (m_specification.layers > 6 && layer != -1)
			{
				spec.baseArrayLayer = layer * 6u;
			}

			spec.viewType = ImageViewType::ViewCube;
		}
		else if (m_specification.layers > 1 && layer == -1)
		{
			if (m_specification.imageType == ResourceType::Image1D)
			{
				spec.viewType = ImageViewType::View1DArray;
			}
			else if (m_specification.imageType == ResourceType::Image2D)
			{
				spec.viewType = ImageViewType::View2DArray;
			}
			else if (m_specification.imageType == ResourceType::Image3D)
			{
				spec.viewType = ImageViewType::View3DArray;
			}
		}

		if (m_specification.isCubeMap && m_specification.layers > 6 && layer == -1 && m_specification.imageType == ResourceType::Image2D)
		{
			spec.viewType = ImageViewType::ViewCubeArray;

			spec.layerCount = m_specification.layers;
		}

		spec.image = this;

		RefPtr<ImageView> view = ImageView::Create(spec);
		m_imageViews[layer][mip] = view;

		return view;
	}

	RefPtr<ImageView> D3D12Image::GetArrayView(const int32_t mip)
	{
		if (m_arrayImageViews.contains(mip))
		{
			return m_arrayImageViews.at(mip);
		}

		ImageViewSpecification spec{};
		spec.baseArrayLayer = 0;
		spec.baseMipLevel = (mip == -1) ? 0 : mip;
		spec.layerCount = m_specification.layers;
		spec.mipCount = (mip == -1) ? m_specification.mips : 1;

		if (m_specification.imageType == ResourceType::Image1D)
		{
			spec.viewType = ImageViewType::View1DArray;
		}
		else if (m_specification.imageType == ResourceType::Image2D)
		{
			spec.viewType = ImageViewType::View2DArray;
		}
		else if (m_specification.imageType == ResourceType::Image3D)
		{
			spec.viewType = ImageViewType::View3DArray;
		}

		spec.image = this;

		RefPtr<ImageView> view = ImageView::Create(spec);
		m_arrayImageViews[mip] = view;

		return view;
	}

	const uint32_t D3D12Image::GetWidth() const
	{
		return m_specification.width;
	}

	const uint32_t D3D12Image::GetHeight() const
	{
		return m_specification.height;
	}

	const uint32_t D3D12Image::GetDepth() const
	{
		return m_specification.depth;
	}

	const uint32_t D3D12Image::GetMipCount() const
	{
		return m_specification.mips;
	}

	const uint32_t D3D12Image::GetLayerCount() const
	{
		return m_specification.layers;
	}

	const PixelFormat D3D12Image::GetFormat() const
	{
		return m_specification.format;
	}

	const ImageUsage D3D12Image::GetUsage() const
	{
		return m_specification.usage;
	}

	const ImageAspect D3D12Image::GetImageAspect() const
	{
		return Utility::IsDepthFormat(m_specification.format) ? ImageAspect::Depth : ImageAspect::Color;
	}

	const uint32_t D3D12Image::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(GetWidth(), GetHeight());
	}

	const bool D3D12Image::IsSwapchainImage() const
	{
		return m_isSwapchainImage;
	}

	void D3D12Image::SetName(std::string_view name)
	{
		m_specification.debugName = name;

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

	std::string_view D3D12Image::GetName() const
	{
		return m_specification.debugName;
	}

	const uint64_t D3D12Image::GetDeviceAddress() const
	{
		return 0;
	}

	const uint64_t D3D12Image::GetByteSize() const
	{
		return m_allocation->GetSize();
	}
}
