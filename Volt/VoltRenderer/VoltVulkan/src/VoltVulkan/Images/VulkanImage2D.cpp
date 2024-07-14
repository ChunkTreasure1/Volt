#include "vkpch.h"
#include "VulkanImage2D.h"

#include "VoltVulkan/Common/VulkanFunctions.h"
#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"
#include "VoltVulkan/Images/VulkanImageView.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Images/ImageView.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Memory/Allocator.h>
#include <VoltRHI/Memory/Allocation.h>

#include <VoltRHI/Utility/ResourceUtility.h>

#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	VulkanImage2D::VulkanImage2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
		: m_specification(specification), m_allocator(allocator)
	{
		if (!allocator)
		{
			m_allocator = GraphicsContext::GetDefaultAllocator();
		}

		Invalidate(specification.width, specification.height, data);
		SetName(specification.debugName);
	}

	VulkanImage2D::VulkanImage2D(const SwapchainImageSpecification& specification)
		: m_isSwapchainImage(true)
	{
		GraphicsContext::GetResourceStateTracker()->AddResource(this, BarrierStage::None, BarrierAccess::None, ImageLayout::Undefined);

		InvalidateSwapchainImage(specification);
		SetName("Swapchain Image");
	}

	VulkanImage2D::~VulkanImage2D()
	{
		GraphicsContext::GetResourceStateTracker()->RemoveResource(this);
		Release();
	}

	void VulkanImage2D::Invalidate(const uint32_t width, const uint32_t height, const void* data)
	{
		Release();

		m_specification.width = width;
		m_specification.height = height;

		VkImageAspectFlags aspectMask = Utility::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_specification.format))
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		m_imageAspect = Utility::GetVoltImageAspect(aspectMask);
		m_allocation = m_allocator->CreateImage(m_specification, m_specification.memoryUsage);

		ImageLayout targetLayout = ImageLayout::Undefined;

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

	void VulkanImage2D::InvalidateSwapchainImage(const SwapchainImageSpecification& specification)
	{
		const auto& vulkanSwapchain = specification.swapchain->AsRef<VulkanSwapchain>();

		m_imageAspect = Utility::GetVoltImageAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_specification.width = vulkanSwapchain.GetWidth();
		m_specification.height = vulkanSwapchain.GetHeight();
		m_specification.format = vulkanSwapchain.GetFormat();
		m_specification.usage = ImageUsage::Attachment;

		m_swapchainImageData.image = vulkanSwapchain.GetImageAtIndex(specification.imageIndex);
	}

	void VulkanImage2D::Release()
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

	void VulkanImage2D::GenerateMips()
	{
		if (m_hasGeneratedMips || m_isSwapchainImage)
		{
			return;
		}

		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();
		commandBuffer->Begin();

		VkCommandBuffer vkCmdBuffer = commandBuffer->GetHandle<VkCommandBuffer>();

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(this);

		// Transition to DST OPTIMAL
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_allocation->GetResourceHandle<VkImage>();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = Utility::GetVkImageLayoutFromImageLayout(currentState.layout);
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;

		const uint32_t mipLevels = CalculateMipCount();
		m_specification.mips = mipLevels;

		vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			for (uint32_t layer = 0; layer < m_specification.layers; layer++)
			{
				// Transfer last mip
				{
					barrier.subresourceRange.baseMipLevel = i - 1;
					barrier.subresourceRange.baseArrayLayer = layer;
					barrier.subresourceRange.levelCount = 1;
					barrier.subresourceRange.layerCount = 1;

					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

					vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
				}

				// Perform blit
				{
					VkImageBlit imageBlit{};
					imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.srcSubresource.layerCount = 1;
					imageBlit.srcSubresource.mipLevel = i - 1;
					imageBlit.srcSubresource.baseArrayLayer = layer;

					imageBlit.srcOffsets[0] = { 0, 0, 0 };
					imageBlit.srcOffsets[1] = { int32_t(m_specification.width >> (i - 1)), int32_t(m_specification.height >> (i - 1)), 1 };

					imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.dstSubresource.layerCount = 1;
					imageBlit.dstSubresource.mipLevel = i;
					imageBlit.dstSubresource.baseArrayLayer = layer;

					imageBlit.dstOffsets[0] = { 0, 0, 0 };
					imageBlit.dstOffsets[1] = { int32_t(m_specification.width >> i), int32_t(m_specification.height >> i), 1 };

					vkCmdBlitImage(vkCmdBuffer, m_allocation->GetResourceHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_allocation->GetResourceHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
				}

				// Transfer last mip back
				{
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

					vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
				}
			}
		}

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = Utility::GetVkImageLayoutFromImageLayout(currentState.layout);
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;

		vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		commandBuffer->End();
		commandBuffer->ExecuteAndWait();

		m_hasGeneratedMips = true;
	}

	const RefPtr<ImageView> VulkanImage2D::GetView(const int32_t mip, const int32_t layer)
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

		RefPtr<ImageView> view = RefPtr<VulkanImageView>::Create(spec);
		m_imageViews[layer][mip] = view;

		return view;
	}

	const RefPtr<ImageView> VulkanImage2D::GetArrayView(const int32_t mip)
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
		spec.viewType = ImageViewType::View2DArray;
		spec.image = As<Image2D>();

		RefPtr<ImageView> view = RefPtr<VulkanImageView>::Create(spec);
		m_arrayImageViews[mip] = view;

		return view;
	}

	const uint32_t VulkanImage2D::GetWidth() const
	{
		return m_specification.width;
	}

	const uint32_t VulkanImage2D::GetHeight() const
	{
		return m_specification.height;
	}

	const uint32_t VulkanImage2D::GetMipCount() const
	{
		return m_specification.mips;
	}

	const uint32_t VulkanImage2D::GetLayerCount() const
	{
		return m_specification.layers;
	}

	const PixelFormat VulkanImage2D::GetFormat() const
	{
		return m_specification.format;
	}

	const ImageUsage VulkanImage2D::GetUsage() const
	{
		return m_specification.usage;
	}

	const uint32_t VulkanImage2D::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(m_specification.width, m_specification.height);
	}

	const bool VulkanImage2D::IsSwapchainImage() const
	{
		return m_isSwapchainImage;
	}

	void VulkanImage2D::SetName(std::string_view name)
	{
		if (!Volt::RHI::vkSetDebugUtilsObjectNameEXT)
		{
			return;
		}

		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		
		if (m_isSwapchainImage)
		{
			nameInfo.objectHandle = (uint64_t)m_swapchainImageData.image;
		}
		else
		{
			nameInfo.objectHandle = (uint64_t)m_allocation->GetResourceHandle<VkImage>();
		}

		nameInfo.pObjectName = name.data();

		auto device = GraphicsContext::GetDevice();
		Volt::RHI::vkSetDebugUtilsObjectNameEXT(device->GetHandle<VkDevice>(), &nameInfo);
	}

	const uint64_t VulkanImage2D::GetDeviceAddress() const
	{
		return m_allocation->GetDeviceAddress();
	}

	const uint64_t VulkanImage2D::GetByteSize() const
	{
		return m_allocation->GetSize();
	}

	void VulkanImage2D::InitializeWithData(const void* data)
	{
		// #TODO_Ivar: Implement correct size for layer + mip
		const VkDeviceSize bufferSize = m_specification.width * m_specification.height * Utility::GetByteSizePerPixelFromFormat(m_specification.format) * m_specification.layers;

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

		commandBuffer->CopyBufferToImage(stagingAlloc, this, m_specification.width, m_specification.height);

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

	void* VulkanImage2D::GetHandleImpl() const
	{
		if (m_isSwapchainImage)
		{
			return m_swapchainImageData.image;
		}
		else
		{
			return m_allocation->GetResourceHandle<VkImage>();
		}
	}

	Buffer VulkanImage2D::ReadPixelInternal(const uint32_t x, const uint32_t y, const size_t stride)
	{
		// #TODO_Ivar: Implement correct size for layer + mip
		const VkDeviceSize bufferSize = m_specification.width * m_specification.height * Utility::GetByteSizePerPixelFromFormat(m_specification.format) * m_specification.layers;

		RefPtr<Allocation> stagingAlloc = GraphicsContext::GetDefaultAllocator()->CreateBuffer(bufferSize, BufferUsage::TransferDst, MemoryUsage::GPUToCPU);

		VkImageAspectFlags aspectFlags = Utility::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_specification.format))
		{
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(this);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_allocation->GetResourceHandle<VkImage>();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = Utility::GetVkImageLayoutFromImageLayout(currentState.layout);
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.subresourceRange.aspectMask = Utility::GetVkImageAspect(m_imageAspect);
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;

		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();

		commandBuffer->Begin();
		vkCmdPipelineBarrier(commandBuffer->GetHandle<VkCommandBuffer>(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		
		region.imageSubresource.aspectMask = aspectFlags;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { m_specification.width, m_specification.height, 1 };

		vkCmdCopyImageToBuffer(commandBuffer->GetHandle<VkCommandBuffer>(), m_allocation->GetResourceHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingAlloc->GetResourceHandle<VkBuffer>(), 1, &region);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = Utility::GetVkImageLayoutFromImageLayout(currentState.layout);
		vkCmdPipelineBarrier(commandBuffer->GetHandle<VkCommandBuffer>(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		commandBuffer->End();
		commandBuffer->ExecuteAndWait();

		uint8_t* mappedMemory = stagingAlloc->Map<uint8_t>();
		
		const uint32_t perPixelSize = Utility::GetByteSizePerPixelFromFormat(m_specification.format);
		const uint32_t bufferIndex = (x + y * m_specification.width) * perPixelSize;

		Buffer buffer{ stride };
		buffer.Copy(&mappedMemory[bufferIndex], stride);
		
		stagingAlloc->Unmap();

		GraphicsContext::GetDefaultAllocator()->DestroyBuffer(stagingAlloc);
		return buffer;
	}

	void VulkanImage2D::TransitionToLayout(ImageLayout targetLayout)
	{
		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();
		commandBuffer->Begin();

		{
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
		}

		commandBuffer->End();
		commandBuffer->Execute();
	}
}
