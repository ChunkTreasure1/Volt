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

#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	namespace Utility
	{
		VkImageLayout ToVulkanLayout(const VulkanImage2D::ImageLayoutInt layout)
		{
			return static_cast<VkImageLayout>(layout);
		}

		uint32_t ToVoltLayout(const VkImageLayout layout)
		{
			return static_cast<uint32_t>(layout);
		}
	}

	VulkanImage2D::VulkanImage2D(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
		: m_specification(specification), m_currentImageLayout(static_cast<uint32_t>(VK_IMAGE_LAYOUT_UNDEFINED)), m_allocator(allocator)
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
		InvalidateSwapchainImage(specification);
		SetName("Swapchain Image");
	}

	VulkanImage2D::~VulkanImage2D()
	{
		Release();
	}

	void VulkanImage2D::Invalidate(const uint32_t width, const uint32_t height, const void* data)
	{
		Release();
		m_currentImageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_UNDEFINED);

		m_specification.width = width;
		m_specification.height = height;

		VkImageAspectFlags aspectMask = Utility::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_specification.format))
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		m_imageAspect = Utility::GetVoltImageAspect(aspectMask);
		m_allocation = m_allocator->CreateImage(m_specification, m_specification.memoryUsage);

		if (data)
		{
			InitializeWithData(data);
		}

		// Transition to correct Layout
		if (m_specification.initializeImage)
		{
			VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			switch (m_specification.usage)
			{
				case ImageUsage::Texture: targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
				case ImageUsage::Storage: targetLayout = VK_IMAGE_LAYOUT_GENERAL; break;

				case ImageUsage::AttachmentStorage:
				case ImageUsage::Attachment:
				{
					if (Utility::IsDepthFormat(m_specification.format))
					{
						if (Utility::IsStencilFormat(m_specification.format))
						{
							targetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						}
						else
						{
							targetLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
						}
					}
					else
					{
						targetLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}

					break;
				}
			}

			TransitionToLayout(Utility::ToVoltLayout(targetLayout));
		}

		if (m_specification.generateMips && m_specification.mips > 1)
		{
			GenerateMips();
		}
	}

	void VulkanImage2D::InvalidateSwapchainImage(const SwapchainImageSpecification& specification)
	{
		const auto& vulkanSwapchain = specification.swapchain->AsRef<VulkanSwapchain>();

		m_currentImageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_UNDEFINED);
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

		// Transition to DST OPTIMAL
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_allocation->GetResourceHandle<VkImage>();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = Utility::ToVulkanLayout(m_currentImageLayout);
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
		barrier.newLayout = Utility::ToVulkanLayout(m_currentImageLayout);
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

	inline const ImageLayout VulkanImage2D::GetImageLayout() const
	{
		return Utility::GetImageLayoutFromVkImageLayout(static_cast<VkImageLayout>(m_currentImageLayout));
	}

	void VulkanImage2D::InitializeWithData(const void* data)
	{
		// #TODO_Ivar: Implement correct size for layer + mip
		const VkDeviceSize bufferSize = m_specification.width * m_specification.height * Utility::GetByteSizePerPixelFromFormat(m_specification.format) * m_specification.layers;

		RefPtr<Allocation> stagingAlloc = GraphicsContext::GetDefaultAllocator()->CreateBuffer(bufferSize, BufferUsage::TransferSrc, MemoryUsage::CPUToGPU);

		auto* stagingData = stagingAlloc->Map<void>();
		memcpy_s(stagingData, bufferSize, data, bufferSize);
		stagingAlloc->Unmap();

		VkImageAspectFlags aspectFlags = Utility::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_specification.format))
		{
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageMemoryBarrier2 barrier2{};
		barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier2.pNext = nullptr;
		barrier2.srcAccessMask = VK_ACCESS_2_NONE;
		barrier2.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier2.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		barrier2.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier2.subresourceRange.aspectMask = Utility::GetVkImageAspect(m_imageAspect);
		barrier2.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier2.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier2.subresourceRange.baseArrayLayer = 0;
		barrier2.subresourceRange.baseMipLevel = 0;
		barrier2.image = m_allocation->GetResourceHandle<VkImage>();

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;
		depInfo.dependencyFlags = 0;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &barrier2;

		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();

		commandBuffer->Begin();
		vkCmdPipelineBarrier2(commandBuffer->GetHandle<VkCommandBuffer>(), &depInfo);

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

		vkCmdCopyBufferToImage(commandBuffer->GetHandle<VkCommandBuffer>(), stagingAlloc->GetResourceHandle<VkBuffer>(), m_allocation->GetResourceHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier2.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		barrier2.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
		barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		barrier2.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

		vkCmdPipelineBarrier2(commandBuffer->GetHandle<VkCommandBuffer>(), &depInfo);

		commandBuffer->End();
		commandBuffer->ExecuteAndWait();

		GraphicsContext::GetDefaultAllocator()->DestroyBuffer(stagingAlloc);

		m_currentImageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_allocation->GetResourceHandle<VkImage>();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
		barrier.newLayout = static_cast<VkImageLayout>(m_currentImageLayout);
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

	void VulkanImage2D::TransitionToLayout(ImageLayoutInt targetLayout)
	{
		if (m_currentImageLayout == targetLayout)
		{
			return;
		}

		VkImageAspectFlags aspectFlags = Utility::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_specification.format))
		{
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_allocation->GetResourceHandle<VkImage>();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = Utility::ToVulkanLayout(m_currentImageLayout);
		barrier.newLayout = Utility::ToVulkanLayout(targetLayout);
		barrier.subresourceRange.aspectMask = Utility::GetVkImageAspect(m_imageAspect);
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;

		RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();

		commandBuffer->Begin();
		vkCmdPipelineBarrier(commandBuffer->GetHandle<VkCommandBuffer>(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		commandBuffer->End();
		commandBuffer->Execute();

		m_currentImageLayout = targetLayout;
	}
}
