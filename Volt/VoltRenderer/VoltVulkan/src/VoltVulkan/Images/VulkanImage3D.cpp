#include "vkpch.h"
#include "VulkanImage3D.h"

#include "VoltVulkan/Common/VulkanFunctions.h"
#include "VoltVulkan/Images/VulkanImageView.h"

#include <VoltRHI/Images/ImageUtility.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Utility/ResourceUtility.h>

#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	VulkanImage3D::VulkanImage3D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
		: m_specification(specification), m_allocator(allocator)
	{
		if (!allocator)
		{
			m_allocator = GraphicsContext::GetDefaultAllocator();
		}

		Invalidate(specification.width, specification.height, specification.depth, data);
		SetName(specification.debugName);
	}

	VulkanImage3D::~VulkanImage3D()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);
	}

	void VulkanImage3D::Invalidate(const uint32_t width, const uint32_t height, const uint32_t depth, const void* data)
	{
		Release();

		m_specification.width = width;
		m_specification.height = height;
		m_specification.depth = depth;

		m_allocation = m_allocator->CreateImage(m_specification, m_specification.memoryUsage);

		ImageLayout targetLayout = ImageLayout::Undefined;

		VT_ASSERT_MSG(m_specification.usage != ImageUsage::Attachment && m_specification.usage != ImageUsage::AttachmentStorage, "Attachment types are not supported for 3D images!");

		switch (m_specification.usage)
		{
			case ImageUsage::Attachment:
			case ImageUsage::AttachmentStorage:
			case ImageUsage::Storage:
			{
				targetLayout = ImageLayout::ShaderWrite;
				break;
			}

			case ImageUsage::Texture:
			{
				targetLayout = ImageLayout::ShaderRead;
				break;
			}
		}

		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None, ImageLayout::Undefined);

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

	void VulkanImage3D::Release()
	{
		m_imageViews.clear();

		if (!m_allocation)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([allocator = m_allocator, allocation = m_allocation] 
		{
			allocator->DestroyImage(allocation);
		});

		m_allocation = nullptr;
	}

	void VulkanImage3D::GenerateMips()
	{
		VT_ENSURE(false);
	}

	const RefPtr<ImageView> VulkanImage3D::GetView(const int32_t mip, const int32_t layer)
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
		spec.viewType = ImageViewType::View3D;

		if (m_specification.layers > 1 && layer == -1)
		{
			spec.viewType = ImageViewType::View3DArray;
		}

		spec.image = this;

		RefPtr<ImageView> view = RefPtr<VulkanImageView>::Create(spec);
		m_imageViews[layer][mip] = view;

		return view;
	}

	const RefPtr<ImageView> VulkanImage3D::GetArrayView(const int32_t mip)
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
		spec.viewType = ImageViewType::View3DArray;
		spec.image = this;

		RefPtr<ImageView> view = RefPtr<VulkanImageView>::Create(spec);
		m_arrayImageViews[mip] = view;

		return view;
	}

	const uint32_t VulkanImage3D::GetWidth() const
	{
		return m_specification.width;
	}

	const uint32_t VulkanImage3D::GetHeight() const
	{
		return m_specification.height;
	}

	const uint32_t VulkanImage3D::GetDepth() const
	{
		return m_specification.depth;
	}

	const uint32_t VulkanImage3D::GetMipCount() const
	{
		return m_specification.mips;
	}

	const uint32_t VulkanImage3D::GetLayerCount() const
	{
		return m_specification.layers;
	}

	const PixelFormat VulkanImage3D::GetFormat() const
	{
		return m_specification.format;
	}

	const ImageUsage VulkanImage3D::GetUsage() const
	{
		return m_specification.usage;
	}

	const uint32_t VulkanImage3D::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(m_specification.width, m_specification.height);
	}

	void VulkanImage3D::SetName(std::string_view name)
	{
		if (!Volt::RHI::vkSetDebugUtilsObjectNameEXT)
		{
			return;
		}

		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		nameInfo.objectHandle = (uint64_t)m_allocation->GetResourceHandle<VkImage>();
		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		Volt::RHI::vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	const uint64_t VulkanImage3D::GetDeviceAddress() const
	{
		return m_allocation->GetDeviceAddress();
	}

	const uint64_t VulkanImage3D::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void* VulkanImage3D::GetHandleImpl() const
	{
		return m_allocation->GetResourceHandle<VkImage>();
	}

	void VulkanImage3D::InitializeWithData(const void* data)
	{
		const VkDeviceSize bufferSize = m_specification.width * m_specification.height * m_specification.depth * Utility::GetByteSizePerPixelFromFormat(m_specification.format) * m_specification.layers;
	
		RefPtr<Allocation> stagingAlloc = GraphicsContext::GetDefaultAllocator()->CreateBuffer(bufferSize, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);

		auto* stagingData = stagingAlloc->Map<void>();
		memcpy_s(stagingData, bufferSize, data, bufferSize);
		stagingAlloc->Unmap();

		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();

		commandBuffer->Begin();

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), this);

			barrier.imageBarrier().dstStage = RHI::BarrierStage::Copy;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::CopyDest;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::CopyDest;
			barrier.imageBarrier().resource = this;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->CopyBufferToImage(stagingAlloc, this, m_specification.width, m_specification.height, m_specification.depth);

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), this);

			barrier.imageBarrier().dstStage = RHI::BarrierStage::PixelShader | RHI::BarrierStage::ComputeShader;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
			barrier.imageBarrier().resource = this;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->End();
		commandBuffer->ExecuteAndWait();

		GraphicsContext::GetDefaultAllocator()->DestroyBuffer(stagingAlloc);
	}

	void VulkanImage3D::TransitionToLayout(ImageLayout targetLayout)
	{
		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();
		commandBuffer->Begin();

		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Image;
		ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), this);

		if (targetLayout == ImageLayout::ShaderRead)
		{
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.imageBarrier().dstStage = RHI::BarrierStage::AllGraphics;
		}
		else if (targetLayout == ImageLayout::ShaderWrite)
		{
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderWrite;
			barrier.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader | RHI::BarrierStage::PixelShader;
		}
		else if (targetLayout == ImageLayout::DepthStencilWrite)
		{
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::DepthStencilWrite;
			barrier.imageBarrier().dstStage = RHI::BarrierStage::DepthStencil;
		}
		else if (targetLayout == ImageLayout::RenderTarget)
		{
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::RenderTarget;
			barrier.imageBarrier().dstStage = RHI::BarrierStage::RenderTarget;
		}

		barrier.imageBarrier().dstLayout = targetLayout;
		barrier.imageBarrier().resource = this;

		commandBuffer->ResourceBarrier({ barrier });

		commandBuffer->End();
		commandBuffer->Execute();
	} 
}
