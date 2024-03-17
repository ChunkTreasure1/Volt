#include "dxpch.h"
#include "D3D12Image2D.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"

#include "VoltRHI/Images/ImageUtility.h"

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



	D3D12Image2D::D3D12Image2D(const ImageSpecification& specification, const void* data) : m_specs(specification)
	{
		m_image = {};

		Invalidate(specification.width, specification.height, data);
	}

	D3D12Image2D::D3D12Image2D(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data)
	{
	}

	D3D12Image2D::~D3D12Image2D()
	{

	}

	void* D3D12Image2D::GetHandleImpl() const 
	{
		return const_cast<void*>(reinterpret_cast<const void*>(&m_image));
	}

	void D3D12Image2D::Invalidate(const uint32_t width, const uint32_t height, const void* data)
	{
		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		D3D12_RESOURCE_DESC desc = {};

		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = ConvertFormatToD3D12Format(m_specs.format);
		desc.MipLevels = static_cast<UINT16>(m_specs.mips);
		desc.Height = m_specs.height;
		desc.Width = m_specs.width;
		desc.DepthOrArraySize = (m_specs.depth == 0) ? 1 : static_cast<UINT16>(m_specs.depth);
		desc.SampleDesc.Count = 1;
		
		desc.Flags = GetFlagFromUsage(m_specs.usage);

		{
			D3D12_HEAP_PROPERTIES HeapProps = {};
			HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			HeapProps.CreationNodeMask = 1;
			HeapProps.VisibleNodeMask = 1;

			VT_D3D12_CHECK(device->CreateCommittedResource(
				&HeapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				nullptr,
				IID_PPV_ARGS(&m_image.texture)
			));
		}/*
		{
			D3D12MA::ALLOCATION_DESC allocDesc = {};
			allocDesc.HeapType = D3d12Allocator::GetHeapTypeFromResourceState(specs.usage);

			D3d12Allocator::Allocate(m_Image, desc, allocDesc, GetD3DResourceState(specs.usage));
		}*/


	}

	void D3D12Image2D::Release()
	{
	}

	void D3D12Image2D::GenerateMips()
	{
	}

	const Ref<ImageView> D3D12Image2D::GetView(const int32_t mip, const int32_t layer)
	{
		if (m_view)
		{
			return m_view;
		}

		ImageViewSpecification viewSpecs = {};

		viewSpecs.image = As<Image2D>();

		m_view = ImageView::Create(viewSpecs);

		return m_view;
	}

	const Ref<ImageView> D3D12Image2D::GetArrayView(const int32_t mip)
	{
		return Ref<ImageView>();
	}

	const uint32_t D3D12Image2D::GetWidth() const
	{
		return m_specs.width;
	}

	const uint32_t D3D12Image2D::GetHeight() const
	{
		return m_specs.height;
	}

	const PixelFormat D3D12Image2D::GetFormat() const
	{
		return m_specs.format;
	}

	const ImageUsage D3D12Image2D::GetUsage() const
	{
		return m_specs.usage;
	}

	const ImageAspect D3D12Image2D::GetImageAspect() const
	{
		return ImageAspect();
	}

	const uint32_t D3D12Image2D::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(GetWidth(), GetHeight());
	}

	void D3D12Image2D::SetName(std::string_view name)
	{
		std::wstring wname(name.begin(), name.end());
		m_image.texture->SetName(wname.c_str());
	}

	const uint64_t D3D12Image2D::GetDeviceAddress() const
	{
		return 0;
	}
	const uint64_t D3D12Image2D::GetByteSize() const
	{
		return 0;
	}
}
