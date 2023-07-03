#pragma once

#include "Volt/Rendering/Texture/ImageCommon.h"
#include "Volt/Rendering/Texture/Image2D.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	namespace Utility
	{
		inline void AccessMasksFromLayouts(VkImageLayout sourceLayout, VkImageLayout targetLayout, VkImageMemoryBarrier& outBarrier)
		{
			// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (sourceLayout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:
					// Image layout is undefined (or does not matter)
					// Only valid as initial layout
					// No flags required, listed only for completeness
					outBarrier.srcAccessMask = 0;
					break;

				case VK_IMAGE_LAYOUT_PREINITIALIZED:
					// Image is preinitialized
					// Only valid as initial layout for linear images, preserves memory contents
					// Make sure host writes have been finished
					outBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					// Image is a color attachment
					// Make sure any writes to the color buffer have been finished
					outBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					// Image is a depth/stencil attachment
					// Make sure any writes to the depth/stencil buffer have been finished
					outBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
					// Image is a depth/stencil attachment
					// Make sure any writes to the depth/stencil buffer have been finished
					outBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					// Image is a transfer source
					// Make sure any reads from the image have been finished
					outBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					// Image is a transfer destination
					// Make sure any writes to the image have been finished
					outBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					// Image is read by a shader
					// Make sure any shader reads from the image have been finished
					outBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
					// Image is read by a shader
					// Make sure any shader reads from the image have been finished
					outBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
					// Image is read by a shader
					// Make sure any shader reads from the image have been finished
					outBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_GENERAL:
					outBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
					break;

				default:
					VT_CORE_ASSERT(false, "Layouts are not configured!");
					break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (targetLayout)
			{
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					// Image will be used as a transfer destination
					// Make sure any writes to the image have been finished
					outBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					// Image will be used as a transfer source
					// Make sure any reads from the image have been finished
					outBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					// Image will be used as a color attachment
					// Make sure any writes to the color buffer have been finished
					outBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
					outBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					// Image layout will be used as a depth/stencil attachment
					// Make sure any writes to depth/stencil buffer have been finished
					outBarrier.dstAccessMask = outBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					// Image will be read in a shader (sampler, input attachment)
					// Make sure any writes to the image have been finished
					outBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
					outBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
					outBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_GENERAL:
					outBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
					break;

				default:
					VT_CORE_ASSERT(false, "Layouts are not configured!");
					break;
			}
		}

		inline std::pair<VkPipelineStageFlags, VkPipelineStageFlags> GetStageFlagsFromLayouts(VkImageLayout currentLayout, VkImageLayout targetLayout)
		{
			VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			VkPipelineStageFlags destStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_GENERAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_GENERAL && targetLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_GENERAL)
			{
				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_GENERAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && targetLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_GENERAL)
			{
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_GENERAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (currentLayout == VK_IMAGE_LAYOUT_GENERAL && targetLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else
			{
				VT_CORE_ASSERT(false, "Layouts are not configured!");
			}

			return { sourceStage, destStage };
		}

		inline void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout, VkImageSubresourceRange subresource)
		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.oldLayout = currentLayout;
			imageMemoryBarrier.newLayout = targetLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresource;

			AccessMasksFromLayouts(currentLayout, targetLayout, imageMemoryBarrier);
			const auto [sourceStage, destStage] = GetStageFlagsFromLayouts(currentLayout, targetLayout);

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		inline void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout)
		{
			VkImageSubresourceRange subresource{};
			subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource.baseMipLevel = 0;
			subresource.levelCount = VK_REMAINING_MIP_LEVELS;
			subresource.baseArrayLayer = 0;
			subresource.layerCount = VK_REMAINING_ARRAY_LAYERS;

			TransitionImageLayout(commandBuffer, image, currentLayout, targetLayout, subresource);
		}

		inline void TransitionImageLayout(VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout)
		{
			auto device = GraphicsContextVolt::GetDevice();

			VkImageSubresourceRange subresource{};
			subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource.baseMipLevel = 0;
			subresource.levelCount = VK_REMAINING_MIP_LEVELS;
			subresource.baseArrayLayer = 0;
			subresource.layerCount = VK_REMAINING_ARRAY_LAYERS;

			auto commandBuffer = device->GetSingleUseCommandBuffer(true);

			TransitionImageLayout(commandBuffer, image, currentLayout, targetLayout, subresource);

			device->FlushSingleUseCommandBuffer(commandBuffer);
		}

		inline void TransitionImageFromTransferQueue(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
		{
			auto device = GraphicsContextVolt::GetDevice();

			VkImageSubresourceRange subresource{};
			subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource.baseMipLevel = 0;
			subresource.levelCount = VK_REMAINING_MIP_LEVELS;
			subresource.baseArrayLayer = 0;
			subresource.layerCount = VK_REMAINING_ARRAY_LAYERS;

			// Release barrier
			{
				VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true, QueueTypeVolt::Transfer);

				VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.srcQueueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().transferFamilyQueueIndex;
				imageMemoryBarrier.dstQueueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().graphicsFamilyQueueIndex;
				imageMemoryBarrier.oldLayout = oldLayout;
				imageMemoryBarrier.newLayout = oldLayout;
				imageMemoryBarrier.image = image;
				imageMemoryBarrier.subresourceRange = subresource;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = 0;

				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

				device->FlushSingleUseCommandBuffer(cmdBuffer);
			}

			// Aquire barrier
			{
				VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);

				VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.srcQueueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().transferFamilyQueueIndex;
				imageMemoryBarrier.dstQueueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().graphicsFamilyQueueIndex;
				imageMemoryBarrier.oldLayout = oldLayout;
				imageMemoryBarrier.newLayout = newLayout;
				imageMemoryBarrier.image = image;
				imageMemoryBarrier.subresourceRange = subresource;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

				device->FlushSingleUseCommandBuffer(cmdBuffer);
			}
		}

		inline static std::vector<VkImageMemoryBarrier> GetImageBarriersFromImages(const std::vector<Ref<Image2D>>& images, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subResourceRange)
		{
			std::vector<VkImageMemoryBarrier> result{};
			for (const auto& img : images)
			{
				auto& barrier = result.emplace_back();
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.srcAccessMask = srcAccessFlags;
				barrier.dstAccessMask = dstAccessFlags;
				barrier.oldLayout = oldLayout;
				barrier.newLayout = newLayout;
				barrier.image = img->GetHandle();
				barrier.subresourceRange = subResourceRange;
			}

			return result;
		}

		void InsertImageMemoryBarriers(VkCommandBuffer commandBuffer, const std::vector<VkImageMemoryBarrier>& barriers, VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags);

		inline void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel = 0)
		{
			VkCommandBuffer cmdBuffer = commandBuffer;

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = mipLevel;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}

		inline void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel = 0)
		{
			auto device = GraphicsContextVolt::GetDevice();

			VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);
			CopyBufferToImage(cmdBuffer, buffer, image, width, height, mipLevel);
			device->FlushSingleUseCommandBuffer(cmdBuffer);
		}

		inline void GenerateMipMaps(VkImage image, uint32_t width, uint32_t height, uint32_t mipLevels)
		{
			auto device = GraphicsContextVolt::GetDevice();
			VkCommandBuffer cmdBuffer = device->GetSingleUseCommandBuffer(true);

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = (int32_t)width;
			int32_t mipHeight = (int32_t)height;

			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				if (mipWidth > 1)
				{
					mipWidth /= 2;
				}

				if (mipHeight > 1)
				{
					mipHeight /= 2;
				}
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmdBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			device->FlushSingleUseCommandBuffer(cmdBuffer);
		}

		inline void InsertImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subResourceRange)
		{
			VkImageMemoryBarrier imageMemBarrier{};
			imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemBarrier.srcAccessMask = srcAccess;
			imageMemBarrier.dstAccessMask = dstAccess;
			imageMemBarrier.oldLayout = oldLayout;
			imageMemBarrier.newLayout = newLayout;
			imageMemBarrier.image = image;
			imageMemBarrier.subresourceRange = subResourceRange;

			vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemBarrier);
		}

		inline void InsertMemoryBarrier(VkCommandBuffer commandBuffer, VkAccessFlags2 srcAccess, VkAccessFlags2 dstAccess, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask)
		{
			VkMemoryBarrier2 memBarrier{};
			memBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
			memBarrier.pNext = nullptr;
			memBarrier.srcStageMask = srcStageMask;
			memBarrier.dstStageMask = dstStageMask;
			memBarrier.srcAccessMask = srcAccess;
			memBarrier.dstAccessMask = dstAccess;

			VkDependencyInfo depInfo{};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.pNext = nullptr;
			depInfo.dependencyFlags = 0;
			depInfo.memoryBarrierCount = 1;
			depInfo.pMemoryBarriers = &memBarrier;
			depInfo.bufferMemoryBarrierCount = 0;
			depInfo.pBufferMemoryBarriers = nullptr;
			depInfo.imageMemoryBarrierCount = 0;
			depInfo.pImageMemoryBarriers = nullptr;
	
			vkCmdPipelineBarrier2(commandBuffer, &depInfo);
		}

		///////////////////////////Conversions////////////////////////////
		inline bool IsDepthFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::DEPTH16U:
				case ImageFormat::DEPTH32F: return true;
				case ImageFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		inline bool IsStencilFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::DEPTH24STENCIL8:
					return true;
			}

			return false;
		}

		inline bool IsFloatFormat(ImageFormat aFormat)
		{
			switch (aFormat)
			{
				case Volt::ImageFormat::R16F:
				case Volt::ImageFormat::R32F:
				case Volt::ImageFormat::RGBA:
				case Volt::ImageFormat::RGBAS:
				case Volt::ImageFormat::RGBA16F:
				case Volt::ImageFormat::RGBA32F:
				case Volt::ImageFormat::RG16F:
				case Volt::ImageFormat::RG32F:
				case Volt::ImageFormat::SRGB:
				case Volt::ImageFormat::BC6H_SF16:
					return true;
			}

			return false;
		}

		inline VkFormat VoltToVulkanFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8U: return VK_FORMAT_R8_UNORM;

				case ImageFormat::R16F: return VK_FORMAT_R16_SFLOAT;

				case ImageFormat::R32F: return VK_FORMAT_R32_SFLOAT;
				case ImageFormat::R32SI: return VK_FORMAT_R32_SINT;
				case ImageFormat::R32UI: return VK_FORMAT_R32_UINT;

				case ImageFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::RGBAS: return VK_FORMAT_R8G8B8A8_SRGB;

				case ImageFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
				case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;

				case ImageFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
				case ImageFormat::RG32F: return VK_FORMAT_R32G32B32_SFLOAT;
				case ImageFormat::RG32UI: return VK_FORMAT_R32G32_UINT;

				case ImageFormat::BC1: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
				case ImageFormat::BC1SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;

				case ImageFormat::BC2: return VK_FORMAT_BC2_UNORM_BLOCK;
				case ImageFormat::BC2SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;

				case ImageFormat::BC3: return VK_FORMAT_BC3_UNORM_BLOCK;
				case ImageFormat::BC3SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;

				case ImageFormat::BC4: return VK_FORMAT_BC4_UNORM_BLOCK;
				case ImageFormat::BC5: return VK_FORMAT_BC5_UNORM_BLOCK;

				case ImageFormat::BC7: return VK_FORMAT_BC7_UNORM_BLOCK;
				case ImageFormat::BC7SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;

				case ImageFormat::DEPTH32F: return VK_FORMAT_D32_SFLOAT;
				case ImageFormat::DEPTH16U: return VK_FORMAT_D16_UNORM;
				case ImageFormat::DEPTH24STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;

				default:
					break;
			}

			VT_CORE_ASSERT(false, "Texture format not supported!");
			return (VkFormat)0;
		}

		inline VkFilter VoltToVulkanFilter(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::None: VT_CORE_ASSERT(false, ""); break;
				case TextureFilter::Linear: return VK_FILTER_LINEAR;
				case TextureFilter::Nearest: return VK_FILTER_NEAREST;
			}

			VT_CORE_ASSERT(false, "");
			return VK_FILTER_LINEAR;
		}

		inline VkSamplerAddressMode VoltToVulkanWrapMode(TextureWrap wrap)
		{
			switch (wrap)
			{
				case TextureWrap::None: VT_CORE_ASSERT(false, ""); break;
				case TextureWrap::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				case TextureWrap::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}

			VT_CORE_ASSERT(false, "");
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}

		inline VkSamplerMipmapMode VolToVulkanMipMapMode(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::None: VT_CORE_ASSERT(false, ""); break;
				case TextureFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
				case TextureFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			}

			VT_CORE_ASSERT(false, "");
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

		inline VkCompareOp VoltToVulkanCompareOp(CompareOperator compareOp)
		{
			switch (compareOp)
			{
				case CompareOperator::None: return VK_COMPARE_OP_NEVER;
				case CompareOperator::Never: return VK_COMPARE_OP_NEVER;
				case CompareOperator::Less: return VK_COMPARE_OP_LESS;
				case CompareOperator::Equal: return VK_COMPARE_OP_EQUAL;
				case CompareOperator::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
				case CompareOperator::Greater: return VK_COMPARE_OP_GREATER;
				case CompareOperator::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
				case CompareOperator::Always: return VK_COMPARE_OP_ALWAYS;
			}

			VT_CORE_ASSERT(false, "Compare operator not supported!");
			return VK_COMPARE_OP_LESS;
		}

		inline static VkAttachmentLoadOp VoltToVulkanLoadOp(ClearMode clearMode)
		{
			switch (clearMode)
			{
				case ClearMode::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
				case ClearMode::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
				case ClearMode::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}

			VT_CORE_ASSERT(false, "Clear mode not supported!");
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		}

		inline uint32_t PerPixelSizeFromFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: return 0;
				
				case ImageFormat::R8U: return 1 * 1;

				case ImageFormat::R16F: return 1 * 2;

				case ImageFormat::R32F: return 1 * 4;
				case ImageFormat::R32SI: return 1 * 4;
				case ImageFormat::R32UI: return 1 * 4;
				
				case ImageFormat::RGBA: return 4 * 1;
				case ImageFormat::RGBA16F: return 4 * 2;
				case ImageFormat::RGBA32F: return 4 * 4;
				case ImageFormat::SRGB: return 4 * 1;
				
				case ImageFormat::RG32UI: return 2 * 4;
				case ImageFormat::RG16F: return 2 * 2;
				case ImageFormat::RG32F: return 2 * 4;

				case ImageFormat::DEPTH32F: return 1 * 4;
				case ImageFormat::DEPTH24STENCIL8: return 4;
				
				case ImageFormat::BC6H_SF16: return 3 * 2;
			}

			return 0;
		}

		inline static uint32_t CalculateMipCount(uint32_t width, uint32_t height)
		{
			return static_cast<uint32_t>(std::floor(std::log2(std::min(width, height)))) + 1;
		}

		inline static uint32_t PreviousPOW2(uint32_t value)
		{
			uint32_t r = 1;

			while (r * 2 < value)
			{
				r *= 2;
			}

			return r;
		}
	}
}
