#include "vtpch.h"
#include "Image3D.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	Image3D::Image3D(const ImageSpecification& specification, const void* data)
		: mySpecification(specification)
	{
		Invalidate(specification.width, specification.height, specification.depth, data);
	}

	Image3D::~Image3D()
	{
		Release();
	}

	void Image3D::Invalidate(uint32_t width, uint32_t height, uint32_t depth, const void* data)
	{
		Release();

		mySpecification.width = width;
		mySpecification.height = height;
		mySpecification.depth = depth;

		myImageData.layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VulkanAllocatorVolt allocator{ };
		//auto device = GraphicsContextVolt::GetDevice();

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
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
		imageInfo.usage = usage;
		imageInfo.extent.width = mySpecification.width;
		imageInfo.extent.height = mySpecification.height;
		imageInfo.extent.depth = mySpecification.depth;
		imageInfo.mipLevels = mySpecification.mips;
		imageInfo.arrayLayers = mySpecification.layers;
		imageInfo.format = myImageData.format;
		imageInfo.flags = 0;

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

		// Create image views
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		viewInfo.format = myImageData.format;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = aspectMask;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = mySpecification.mips;
		viewInfo.subresourceRange.layerCount = mySpecification.layers;
		viewInfo.image = myImage;

		//VT_VK_CHECK(vkCreateImageView(device->GetHandle(), &viewInfo, nullptr, &myImageViews[0]));

		// Transition to correct layout
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

		//auto commandBuffer = device->GetSingleUseCommandBuffer(true);
		//TransitionToLayout(commandBuffer, targetLayout);
		//device->FlushSingleUseCommandBuffer(commandBuffer);

		myDescriptorInfo.imageLayout = myImageData.layout;
		myDescriptorInfo.imageView = myImageViews.at(0);
		myDescriptorInfo.sampler = myImageData.sampler;
	}

	void Image3D::Release()
	{
		VT_PROFILE_FUNCTION();

		if (!myImage && !myAllocation)
		{
			return;
		}

		//Renderer::SubmitResourceChange([imageViews = myImageViews, image = myImage, allocation = myAllocation]()
		//{
		//	auto device = GraphicsContextVolt::GetDevice();
		//	for (auto& [mip, view] : imageViews)
		//	{
		//		vkDestroyImageView(device->GetHandle(), view, nullptr);
		//	}

		//	VulkanAllocatorVolt allocator{ "Image2D - Destroy" };
		//	allocator.DestroyImage(image, allocation);
		//});

		myImageViews.clear();
		myImage = nullptr;
		myAllocation = nullptr;
	}

	void Image3D::TransitionToLayout(VkCommandBuffer commandBuffer, VkImageLayout targetLayout)
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

	void Image3D::CreateMipViews()
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

	VkImageView Image3D::CreateMipView(const uint32_t mip)
	{
		if (myImageViews.find(mip) != myImageViews.end())
		{
			return myImageViews.at(mip);
		}

		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_3D;
		info.format = myImageData.format;
		info.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = mip;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = mySpecification.layers;
		info.subresourceRange.levelCount = 1;
		info.image = myImage;

		//auto device = GraphicsContextVolt::GetDevice();
		//VT_VK_CHECK(vkCreateImageView(device->GetHandle(), &info, nullptr, &myImageViews[mip]));

		return myImageViews.at(mip);
	}

	VkImageAspectFlags Image3D::GetImageAspect() const
	{
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Utility::IsStencilFormat(mySpecification.format))
		{
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		return aspectFlags;
	}

	void Image3D::SetName(const std::string& name)
	{
		//GraphicsContextVolt::SetImageName(myImage, name);
	}

	Ref<Image3D> Image3D::Create(const ImageSpecification& specification, const void* data)
	{
		return CreateRef<Image3D>(specification, data);
	}
}
