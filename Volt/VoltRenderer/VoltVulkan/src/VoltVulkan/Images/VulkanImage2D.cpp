#include "vkpch.h"
#include "VulkanImage2D.h"

#include "VoltVulkan/Graphics/VulkanAllocator.h"
#include "VoltVulkan/Common/VulkanHelpers.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Graphics/GraphicsContext.h>

namespace Volt::RHI
{
	namespace Utility
	{
		VkImageLayout ToVulkanLayout(const VulkanImage2D::ImageLayout layout)
		{
			return static_cast<VkImageLayout>(layout);
		}

		uint32_t ToVoltLayout(const VkImageLayout layout)
		{
			return static_cast<uint32_t>(layout);
		}
	}

	VulkanImage2D::VulkanImage2D(const ImageSpecification& specification, const void* data)
		: m_specification(specification), m_currentImageLayout(static_cast<uint32_t>(VK_IMAGE_LAYOUT_UNDEFINED))
	{
		Invalidate(specification.width, specification.height, data);
	}

	VulkanImage2D::~VulkanImage2D()
	{
		Release();
	}

	void VulkanImage2D::Invalidate(const uint32_t width, const uint32_t height, const void* data)
	{
		Release();

		m_specification.width = width;
		m_specification.height = height;

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (m_specification.usage == ImageUsage::Attachment)
		{
			usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			if (Utility::IsDepthFormat(m_specification.format))
			{
				usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}
		else if (m_specification.usage == ImageUsage::AttachmentStorage)
		{
			usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			if (Utility::IsDepthFormat(m_specification.format))
			{
				usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}
		else if (m_specification.usage == ImageUsage::Texture)
		{
			usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		else if (m_specification.usage == ImageUsage::Storage)
		{
			usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		}

		VkImageAspectFlags aspectMask = Utility::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (Utility::IsStencilFormat(m_specification.format))
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.usage = usageFlags;
		imageInfo.extent.width = m_specification.width;
		imageInfo.extent.height = m_specification.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = m_specification.mips;
		imageInfo.arrayLayers = m_specification.layers;
		imageInfo.format = Utility::VoltToVulkanFormat(m_specification.format);
		imageInfo.flags = 0;

		if (m_specification.isCubeMap)
		{
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		switch (m_specification.memoryUsage)
		{
			case MemoryUsage::CPUToGPU:
				memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				break;
			case MemoryUsage::GPUOnly:
				memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
				break;
		}

		VulkanAllocator allocator{};
		m_allocation = allocator.AllocateImage(imageInfo, memUsage, m_image, m_specification.debugName);

		if (data)
		{
			// #TODO_Ivar: Implement initializing using data
		}

		// Transition to correct Layout
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

	void VulkanImage2D::Release()
	{
		if (!m_image)
		{
			return;
		}

		// #TODO_Ivar: Move to deletion queue
		m_imageViews.clear();

		VulkanAllocator allocator{};
		allocator.DestroyImage(m_image, m_allocation);

		m_image = nullptr;
		m_allocation = nullptr;
	}

	void VulkanImage2D::GenerateMips()
	{
		if (m_hasGeneratedMips)
		{
			return;
		}

		Ref<CommandBuffer> commandBuffer = CommandBuffer::Create();
		commandBuffer->Begin();

		VkCommandBuffer vkCmdBuffer = commandBuffer->GetHandle<VkCommandBuffer>();

		// Transition to DST OPTIMAL
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_image;
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

					vkCmdBlitImage(vkCmdBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
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

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = Utility::ToVulkanLayout(m_currentImageLayout);
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;

		vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		commandBuffer->End();
		commandBuffer->Execute();

		m_hasGeneratedMips = true;
	}

	const Ref<ImageView> VulkanImage2D::GetView(const int32_t mip, const int32_t layer)
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
			spec.viewType = ImageViewType::ViewCube;
		}
		else if (m_specification.layers > 1)
		{
			spec.viewType = ImageViewType::View2DArray;
		}

		if (m_specification.isCubeMap && m_specification.layers > 6)
		{
			spec.viewType = ImageViewType::ViewCubeArray;
		}

		spec.viewType = ImageViewType::View2D;
		spec.image = As<Image2D>();

		Ref<ImageView> view = ImageView::Create(spec);
		m_imageViews[layer][mip] = view;

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

	const Format VulkanImage2D::GetFormat() const
	{
		return m_specification.format;
	}

	const uint32_t VulkanImage2D::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(m_specification.width, m_specification.height);
	}

	void* VulkanImage2D::GetHandleImpl()
	{
		return m_image;
	}

	void VulkanImage2D::TransitionToLayout(ImageLayout targetLayout)
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
		barrier.image = m_image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = Utility::ToVulkanLayout(m_currentImageLayout);
		barrier.newLayout = Utility::ToVulkanLayout(targetLayout);
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;

		Ref<CommandBuffer> commandBuffer = CommandBuffer::Create();

		commandBuffer->Begin();
		vkCmdPipelineBarrier(commandBuffer->GetHandle<VkCommandBuffer>(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	
		commandBuffer->End();
		commandBuffer->Execute();
	}
}
