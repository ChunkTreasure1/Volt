#include "vtpch.h"
#include "Image2D.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	Image2D::Image2D(const ImageSpecification& specification, const void* data)
		: mySpecification(specification)
	{
		Invalidate(specification.width, specification.height, true, data);
	}

	Image2D::Image2D(const ImageSpecification& specification, bool transitionTolayout)
		: mySpecification(specification)
	{
		Invalidate(specification.width, specification.height, transitionTolayout, nullptr);
	}

	Image2D::~Image2D()
	{
		Release();
	}

	void Image2D::Invalidate(uint32_t width, uint32_t height, bool transitionLayout, const void* data)
	{
		VT_PROFILE_FUNCTION();

		Release();

		mySpecification.width = width;
		mySpecification.height = height;

		myImageData.layout = VK_IMAGE_LAYOUT_UNDEFINED;

		std::string allocatorName = "Image2D - Create";

		VulkanAllocator allocator{ allocatorName };
		auto device = GraphicsContextVolt::GetDevice();

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (mySpecification.usage == ImageUsage::Attachment)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			if (Utility::IsDepthFormat(mySpecification.format))
			{
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}
		else if (mySpecification.usage == ImageUsage::AttachmentStorage)
		{
			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			if (Utility::IsDepthFormat(mySpecification.format))
			{
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}
		else if (mySpecification.usage == ImageUsage::Texture)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		else if (mySpecification.usage == ImageUsage::Storage)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		}

		VkImageAspectFlags aspectMask = Utility::IsDepthFormat(mySpecification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (mySpecification.format == ImageFormat::DEPTH24STENCIL8)
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		myImageData.format = Utility::VoltToVulkanFormat(mySpecification.format);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.usage = usage;
		imageInfo.extent.width = mySpecification.width;
		imageInfo.extent.height = mySpecification.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mySpecification.mips;
		imageInfo.arrayLayers = mySpecification.layers;
		imageInfo.format = myImageData.format;
		imageInfo.flags = 0;

		if (mySpecification.isCubeMap)
		{
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		switch (mySpecification.memoryUsage)
		{
			case MemoryUsage::CPUToGPU:
				memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				break;
			case MemoryUsage::GPUOnly:
				memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
				break;
			default:
				break;
		}

		myAllocation = allocator.AllocateImage(imageInfo, memUsage, myImage, std::format("Image {}", mySpecification.debugName));
		SetName(mySpecification.debugName);

		if (mySpecification.mappable)
		{
			const VkDeviceSize bufferSize = mySpecification.width * mySpecification.height * Utility::PerPixelSizeFromFormat(mySpecification.format);

			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			myMappingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, myMappingBuffer);
		}

		if (data)
		{
			VkBuffer stagingBuffer;
			VmaAllocation stagingAllocation;
			const VkDeviceSize bufferSize = mySpecification.width * mySpecification.height * Utility::PerPixelSizeFromFormat(mySpecification.format);

			// #TODO_Ivar: Implement correct size for layer + mip

			// Staging buffer
			{
				VkBufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				info.size = bufferSize;
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				stagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

				auto* stagingData = allocator.MapMemory<void>(stagingAllocation);
				memcpy_s(stagingData, bufferSize, data, bufferSize);
				allocator.UnmapMemory(stagingAllocation);

				Utility::TransitionImageLayout(myImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				Utility::CopyBufferToImage(stagingBuffer, myImage, mySpecification.width, mySpecification.height);
				Utility::TransitionImageLayout(myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				allocator.DestroyBuffer(stagingBuffer, stagingAllocation);
			}
		}

		// Create image views
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		if (mySpecification.isCubeMap)
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else if (mySpecification.layers > 1)
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}

		if (mySpecification.isCubeMap && mySpecification.layers > 6)
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}

		viewInfo.format = myImageData.format;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = aspectMask;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = mySpecification.mips;
		viewInfo.subresourceRange.layerCount = mySpecification.layers;
		viewInfo.image = myImage;

		VT_VK_CHECK(vkCreateImageView(device->GetHandle(), &viewInfo, nullptr, &myImageViews[0]));

		if (mySpecification.isCubeMap)
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			VT_VK_CHECK(vkCreateImageView(device->GetHandle(), &viewInfo, nullptr, &myArrayImageViews[0]));
		}

		if (mySpecification.generateMips && mySpecification.mips > 1)
		{
			GenerateMips();
		}

		// Transition to correct layout
		if (transitionLayout)
		{
			VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			switch (mySpecification.usage)
			{
				case ImageUsage::Texture: targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
				case ImageUsage::Storage: targetLayout = VK_IMAGE_LAYOUT_GENERAL; break;

				case ImageUsage::AttachmentStorage:
				case ImageUsage::Attachment:
				{
					// These are the read only layouts because the framebuffer class assumes they are in read only,
					// even on the first bind
					if (Utility::IsDepthFormat(mySpecification.format))
					{
						if (Utility::IsStencilFormat(mySpecification.format))
						{
							targetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						}
						else
						{
							targetLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
						}
					}
					else
					{
						targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					}
					break;
				}
			}

			auto commandBuffer = device->GetSingleUseCommandBuffer(true);
			TransitionToLayout(commandBuffer, targetLayout);
			device->FlushSingleUseCommandBuffer(commandBuffer);
		}

		myDescriptorInfo.imageLayout = myImageData.layout;
		myDescriptorInfo.imageView = myImageViews.at(0);
		myDescriptorInfo.sampler = myImageData.sampler;
	}

	void Image2D::Release()
	{
		VT_PROFILE_FUNCTION();

		if (!myImage && !myAllocation || myAllocatedWithCustomPool)
		{
			return;
		}

		Renderer::SubmitResourceChange([imageViews = myImageViews, arrayViews = myArrayImageViews, image = myImage, allocation = myAllocation, usedCustomPool = myAllocatedWithCustomPool, mappingAlloc = myMappingAllocation, mappingBuffer = myMappingBuffer]()
		{
			auto device = GraphicsContextVolt::GetDevice();
			for (auto& [mip, view] : imageViews)
			{
				vkDestroyImageView(device->GetHandle(), view, nullptr);
			}

			for (auto& [mip, view] : arrayViews)
			{
				vkDestroyImageView(device->GetHandle(), view, nullptr);
			}

			if (!usedCustomPool)
			{
				VulkanAllocator allocator{ "Image2D - Destroy" };
				allocator.DestroyImage(image, allocation);
			}

			if (mappingBuffer)
			{
				VulkanAllocator allocator{ "Image2D - Destroy" };
				allocator.DestroyBuffer(mappingBuffer, mappingAlloc);
			}
		});

		myImageViews.clear();
		myImage = nullptr;
		myAllocation = nullptr;
	}

	void Image2D::TransitionToLayout(VkCommandBuffer commandBuffer, VkImageLayout targetLayout)
	{
		if (myImageData.layout == targetLayout)
		{
			return;
		}

		VkImageAspectFlags flags = Utility::IsDepthFormat(mySpecification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (mySpecification.format == ImageFormat::DEPTH24STENCIL8)
		{
			flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		VkImageSubresourceRange range{};
		range.aspectMask = flags;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = mySpecification.layers;
		range.levelCount = mySpecification.mips;

		Utility::TransitionImageLayout(commandBuffer, myImage, myImageData.layout, targetLayout, range);
		myImageData.layout = targetLayout;
		myDescriptorInfo.imageLayout = targetLayout;
	}

	void Image2D::GenerateMips(bool readOnly, VkCommandBuffer commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContextVolt::GetDevice();
		VkCommandBuffer cmdBuffer = nullptr;

		if (!commandBuffer)
		{
			cmdBuffer = device->GetSingleUseCommandBuffer(true);
		}
		else
		{
			cmdBuffer = commandBuffer;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = myImage;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = myImageData.layout;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.layerCount = 1;

		const uint32_t mipLevels = CalculateMipCount();
		mySpecification.mips = mipLevels;

		for (uint32_t layer = 0; layer < mySpecification.layers; layer++)
		{
			barrier.subresourceRange.baseArrayLayer = layer;
			vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			for (uint32_t layer = 0; layer < mySpecification.layers; layer++)
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

					vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
				}

				// Perform blit
				{
					VkImageBlit imageBlit{};
					imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.srcSubresource.layerCount = 1;
					imageBlit.srcSubresource.mipLevel = i - 1;
					imageBlit.srcSubresource.baseArrayLayer = layer;

					imageBlit.srcOffsets[0] = { 0, 0, 0 };
					imageBlit.srcOffsets[1] = { int32_t(mySpecification.width >> (i - 1)), int32_t(mySpecification.height >> (i - 1)), 1 };

					imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.dstSubresource.layerCount = 1;
					imageBlit.dstSubresource.mipLevel = i;
					imageBlit.dstSubresource.baseArrayLayer = layer;

					imageBlit.dstOffsets[0] = { 0, 0, 0 };
					imageBlit.dstOffsets[1] = { int32_t(mySpecification.width >> i), int32_t(mySpecification.height >> i), 1 };

					vkCmdBlitImage(cmdBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
				}

				// Transfer last mip back
				{
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

					vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
				}
			}
		}

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.layerCount = mySpecification.layers;
		subresourceRange.levelCount = mySpecification.mips;

		const VkImageLayout targetLayout = readOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;

		Utility::TransitionImageLayout(cmdBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetLayout, subresourceRange);

		if (!commandBuffer)
		{
			device->FlushSingleUseCommandBuffer(cmdBuffer);
		}

		myHasGeneratedMips = true;
		myImageData.layout = targetLayout;
		myDescriptorInfo.imageLayout = targetLayout;
	}

	void Image2D::CreateMipViews()
	{
		if (myImageViews.size() > 1)
		{
			return;
		}

		for (uint32_t i = 1; i < mySpecification.mips; i++)
		{
			CreateMipView(i);
		}
	}

	const uint32_t Image2D::CalculateMipCount() const
	{
		return Utility::CalculateMipCount(mySpecification.width, mySpecification.height);
	}

	const VkDescriptorImageInfo& Image2D::GetDescriptorInfo(VkImageLayout targetLayout)
	{
		myDescriptorInfo.imageLayout = targetLayout;
		return myDescriptorInfo;
	}

	void Image2D::CopyFromImage(VkCommandBuffer commandBuffer, Ref<Image2D> srcImage)
	{
		const auto srcOriginalLayout = srcImage->GetLayout();
		const auto dstOriginalLayout = GetLayout();

		srcImage->TransitionToLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		TransitionToLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageCopy region{};
		region.srcSubresource.aspectMask = Utility::IsDepthFormat(mySpecification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = mySpecification.layers;
		region.srcSubresource.mipLevel = 0;

		region.dstSubresource.aspectMask = Utility::IsDepthFormat(srcImage->GetSpecification().format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = srcImage->GetSpecification().layers;
		region.dstSubresource.mipLevel = 0;

		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent = { mySpecification.width, mySpecification.height, 1 };

		vkCmdCopyImage(commandBuffer, srcImage->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		srcImage->TransitionToLayout(commandBuffer, srcOriginalLayout);
		TransitionToLayout(commandBuffer, dstOriginalLayout);
	}

	VkImageView Image2D::CreateMipView(const uint32_t mip)
	{
		if (myImageViews.find(mip) != myImageViews.end())
		{
			return myImageViews.at(mip);
		}

		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;

		if (mySpecification.isCubeMap)
		{
			info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else if (mySpecification.layers > 1)
		{
			info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
		else if (mySpecification.isCubeMap && mySpecification.layers > 6)
		{
			info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}

		info.format = myImageData.format;
		info.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = mip;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = mySpecification.layers;
		info.subresourceRange.levelCount = 1;
		info.image = myImage;

		auto device = GraphicsContextVolt::GetDevice();
		VT_VK_CHECK(vkCreateImageView(device->GetHandle(), &info, nullptr, &myImageViews[mip]));

		if (mySpecification.isCubeMap)
		{
			info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			VT_VK_CHECK(vkCreateImageView(device->GetHandle(), &info, nullptr, &myArrayImageViews[mip]));
		}

		return myImageViews.at(mip);
	}

	VkImageAspectFlags Image2D::GetImageAspect() const
	{
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Utility::IsStencilFormat(mySpecification.format))
		{
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		return aspectFlags;
	}

	void Image2D::SetName(const std::string& name)
	{
		GraphicsContextVolt::SetImageName(myImage, name);
	}

	void Image2D::Unmap()
	{
		VulkanAllocator allocator{};
		allocator.UnmapMemory(myMappingAllocation);

		VkCommandBuffer commandBuffer = GraphicsContext::GetDevice()->GetCommandBuffer(true);

		Utility::TransitionImageLayout(commandBuffer, myImage, myImageData.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(commandBuffer, myMappingBuffer, myImage, mySpecification.width, mySpecification.height);
		Utility::TransitionImageLayout(commandBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, myImageData.layout);

		GraphicsContext::GetDevice()->FlushCommandBuffer(commandBuffer);
	}

	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data)
	{
		return CreateRef<Image2D>(specification, data);
	}

	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, bool transitionLayout)
	{
		return CreateRef<Image2D>(specification, transitionLayout);
	}

	Buffer Image2D::ReadPixelInternal(uint32_t x, uint32_t y, uint32_t size)
	{
		VulkanAllocator allocator{ "Image2D - Read Pixel" };

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		const VkDeviceSize bufferSize = mySpecification.width * mySpecification.height * Utility::PerPixelSizeFromFormat(mySpecification.format);

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		stagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

		VkCommandBuffer commandBuffer = GraphicsContextVolt::GetDevice()->GetSingleUseCommandBuffer(true);

		auto originalLayout = myImageData.layout;
		TransitionToLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageSubresourceLayers subResourceLayers{};
		subResourceLayers.aspectMask = Utility::IsDepthFormat(mySpecification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceLayers.mipLevel = 0;
		subResourceLayers.baseArrayLayer = 0;
		subResourceLayers.layerCount = 1;

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = subResourceLayers;
		region.imageOffset.x = 0;
		region.imageOffset.y = 0;
		region.imageOffset.z = 0;
		region.imageExtent = { mySpecification.width, mySpecification.height, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
		TransitionToLayout(commandBuffer, originalLayout);

		GraphicsContextVolt::GetDevice()->FlushSingleUseCommandBuffer(commandBuffer);

		uint8_t* mappedMemory = allocator.MapMemory<uint8_t>(stagingAllocation);

		const uint32_t perPixelSize = Utility::PerPixelSizeFromFormat(mySpecification.format);
		const uint32_t bufferIndex = (x + y * mySpecification.width) * perPixelSize;
		Buffer buffer{ size };
		buffer.Copy(&mappedMemory[bufferIndex], size);

		allocator.UnmapMemory(stagingAllocation);
		allocator.DestroyBuffer(stagingBuffer, stagingAllocation);

		return buffer;
	}

	Buffer Image2D::ReadPixelRangeInternal(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY, uint32_t size)
	{
		VulkanAllocator allocator{ "Image2D - Read Pixel" };

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		const VkDeviceSize bufferSize = mySpecification.width * mySpecification.height * Utility::PerPixelSizeFromFormat(mySpecification.format);

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		stagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

		VkCommandBuffer commandBuffer = GraphicsContextVolt::GetDevice()->GetSingleUseCommandBuffer(true);

		auto originalLayout = myImageData.layout;
		TransitionToLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageSubresourceLayers subResourceLayers{};
		subResourceLayers.aspectMask = Utility::IsDepthFormat(mySpecification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceLayers.mipLevel = 0;
		subResourceLayers.baseArrayLayer = 0;
		subResourceLayers.layerCount = 1;

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = subResourceLayers;
		region.imageOffset.x = 0;
		region.imageOffset.y = 0;
		region.imageOffset.z = 0;
		region.imageExtent = { mySpecification.width, mySpecification.height, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
		TransitionToLayout(commandBuffer, originalLayout);

		GraphicsContextVolt::GetDevice()->FlushSingleUseCommandBuffer(commandBuffer);

		uint8_t* mappedMemory = allocator.MapMemory<uint8_t>(stagingAllocation);

		const uint32_t perPixelSize = Utility::PerPixelSizeFromFormat(mySpecification.format);

		Buffer buffer{ (maxX - minX) * (maxY - minY) * size };
		uint32_t offset = 0;

		for (uint32_t x = minX; x < maxX; x++)
		{
			for (uint32_t y = minY; y < maxY; y++)
			{
				uint32_t loc = (x + y * mySpecification.width * perPixelSize);
				buffer.Copy(&mappedMemory[loc], size, offset);
				offset += size;
			}
		}

		allocator.UnmapMemory(stagingAllocation);
		allocator.DestroyBuffer(stagingBuffer, stagingAllocation);

		return buffer;
	}

	void* Image2D::MapInternal()
	{
		VulkanAllocator allocator{};
		return allocator.MapMemory<void>(myMappingAllocation);
	}
}
