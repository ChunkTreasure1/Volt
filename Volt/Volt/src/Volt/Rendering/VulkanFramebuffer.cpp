#include "vtpch.h"
#include "VulkanFramebuffer.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebufferSpecification& specification)
		: mySpecification(specification)
	{
		VT_CORE_ASSERT(!specification.attachments.empty(), "Framebuffer requires atleast 1 attachment!");
		memset(&myDepthAttachmentInfo, 0, sizeof(VkRenderingAttachmentInfo));

		myWidth = mySpecification.width;
		myHeight = mySpecification.height;

		for (uint32_t attachmentIndex = 0; auto & attachment : mySpecification.attachments)
		{
			if (mySpecification.existingImages.contains(attachmentIndex))
			{
				if (Utility::IsDepthFormat(mySpecification.existingImages.at(attachmentIndex)->GetFormat()))
				{
					myDepthAttachmentImage = mySpecification.existingDepth;
				}
				else
				{
					myColorAttachmentImages.emplace_back();
				}
			}
			else if (Utility::IsDepthFormat(attachment.format) && !myDepthAttachmentImage)
			{
				ImageSpecification spec{};
				spec.format = attachment.format;
				spec.usage = attachment.storageCompatible ? ImageUsage::AttachmentStorage : ImageUsage::Attachment;
				spec.width = myWidth;
				spec.height = myHeight;
				spec.isCubeMap = attachment.isCubeMap;
				spec.layers = attachment.layers;

				myDepthAttachmentImage = Image2D::Create(spec);
			}
			else
			{
				ImageSpecification spec{};
				spec.format = attachment.format;
				spec.usage = attachment.storageCompatible ? ImageUsage::AttachmentStorage : ImageUsage::Attachment;
				spec.width = myWidth;
				spec.height = myHeight;
				spec.isCubeMap = attachment.isCubeMap;
				spec.layers = attachment.layers;

				myColorAttachmentImages.emplace_back(Image2D::Create(spec));
			}

			attachmentIndex++;
		}

		Invalidate();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		Release();
	}

	void VulkanFramebuffer::Invalidate()
	{
		Release();

		auto device = GraphicsContext::GetDevice();
		const bool createImages = myColorAttachmentImages.empty();

		if (!createImages)
		{
			myColorFormats.resize(myColorAttachmentImages.size());
			myColorAttachmentInfos.resize(myColorAttachmentImages.size());
		}

		for (uint32_t attachmentIndex = 0; auto & attachment : mySpecification.attachments)
		{
			if (Utility::IsDepthFormat(attachment.format))
			{
				if (!mySpecification.existingDepth)
				{
					myDepthAttachmentImage->Invalidate(myWidth, myHeight);
				}

				myDepthFormat = Utility::VoltToVulkanFormat(myDepthAttachmentImage->GetFormat());
				myDepthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				myDepthAttachmentInfo.imageView = myDepthAttachmentImage->GetView();

				if (Utility::IsStencilFormat(myDepthAttachmentImage->GetFormat()))
				{
					myDepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					myDepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				}

				myDepthAttachmentInfo.loadOp = mySpecification.existingDepth ? VK_ATTACHMENT_LOAD_OP_LOAD : Utility::VoltToVulkanLoadOp(attachment.clearMode);
				myDepthAttachmentInfo.clearValue.depthStencil = { 1.f, 0 };
			}
			else
			{
				Ref<Image2D> colorAttachment;
				bool existingImage = false;

				if (mySpecification.existingImages.contains(attachmentIndex))
				{
					colorAttachment = mySpecification.existingImages.at(attachmentIndex);
					myColorAttachmentImages[attachmentIndex] = colorAttachment;
					existingImage = true;
				}
				else
				{
					myColorAttachmentImages.at(attachmentIndex)->Invalidate(myWidth, myHeight);
					colorAttachment = myColorAttachmentImages.at(attachmentIndex);
				}

				myColorFormats.at(attachmentIndex) = Utility::VoltToVulkanFormat(colorAttachment->GetFormat());

				auto& attInfo = myColorAttachmentInfos.at(attachmentIndex);
				attInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				attInfo.imageView = colorAttachment->GetView();
				attInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attInfo.loadOp = existingImage ? VK_ATTACHMENT_LOAD_OP_LOAD : Utility::VoltToVulkanLoadOp(attachment.clearMode);
				attInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attInfo.clearValue = { attachment.clearColor.x, attachment.clearColor.y, attachment.clearColor.z, attachment.clearColor.w };
			}

			attachmentIndex++;
		}
	}

	void VulkanFramebuffer::Bind(VkCommandBuffer commandBuffer) const
	{
		VkExtent2D extent{};
		extent.width = GetWidth();
		extent.height = GetHeight();

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;

		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor = { { 0, 0 }, extent };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Transition color attachments
		{
			const auto barriers = Utility::GetImageBarriersFromImages(myColorAttachmentImages,
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

			Utility::InsertImageMemoryBarriers(commandBuffer, barriers, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}

		// Transition depth attachment
		{
			const VkImageLayout newLayout = Utility::IsStencilFormat(myDepthAttachmentImage->GetFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

			Utility::InsertImageMemoryBarrier(commandBuffer, myDepthAttachmentImage->GetHandle(),
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, newLayout,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkImageSubresourceRange{ myDepthAttachmentImage->GetImageAspect(), 0, 1, 0, 1 });
		}
	}

	void VulkanFramebuffer::Unbind(VkCommandBuffer commandBuffer) const
	{
		// Transition color attachments
		{
			const auto barriers = Utility::GetImageBarriersFromImages(myColorAttachmentImages,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

			Utility::InsertImageMemoryBarriers(commandBuffer, barriers, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		}

		// Transition depth attachment
		{
			const VkImageLayout newLayout = Utility::IsStencilFormat(myDepthAttachmentImage->GetFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			const VkImageLayout oldLayout = Utility::IsStencilFormat(myDepthAttachmentImage->GetFormat()) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

			Utility::InsertImageMemoryBarrier(commandBuffer, myDepthAttachmentImage->GetHandle(),
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
				oldLayout, newLayout,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VkImageSubresourceRange{ myDepthAttachmentImage->GetImageAspect(), 0, 1, 0, 1 });
		}
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		myWidth = width;
		myHeight = height;

		GraphicsContext::GetDevice()->WaitForIdle();
		Invalidate();
	}

	void VulkanFramebuffer::Clear(VkCommandBuffer commandBuffer)
	{
		for (uint32_t index = 0; const auto & att : myColorAttachmentImages)
		{
			if (!mySpecification.existingImages.contains(index))
			{
				const auto& spec = mySpecification.attachments.at(index);

				VkClearColorValue clearColor{};
				clearColor.float32[0] = spec.clearColor.x;
				clearColor.float32[1] = spec.clearColor.y;
				clearColor.float32[2] = spec.clearColor.z;
				clearColor.float32[3] = spec.clearColor.w;

				VkImageSubresourceRange range{};
				range.aspectMask = att->GetImageAspect();
				range.baseArrayLayer = 0;
				range.baseMipLevel = 0;
				range.levelCount = VK_REMAINING_MIP_LEVELS;
				range.layerCount = VK_REMAINING_ARRAY_LAYERS;

				vkCmdClearColorImage(commandBuffer, att->GetHandle(), att->GetLayout(), &clearColor, 1, &range);
			}

			index++;
		}

		if (!mySpecification.existingDepth && myDepthAttachmentImage)
		{
			VkClearDepthStencilValue clearColor{};
			clearColor.depth = 1.f;
			clearColor.stencil = 0;

			VkImageSubresourceRange range{};
			range.aspectMask = myDepthAttachmentImage->GetImageAspect();
			range.baseArrayLayer = 0;
			range.baseMipLevel = 0;
			range.levelCount = VK_REMAINING_MIP_LEVELS;
			range.layerCount = VK_REMAINING_ARRAY_LAYERS;

			vkCmdClearDepthStencilImage(commandBuffer, myDepthAttachmentImage->GetHandle(), myDepthAttachmentImage->GetLayout(), &clearColor, 1, &range);
		}
	}

	Ref<VulkanFramebuffer> VulkanFramebuffer::Create(const VulkanFramebufferSpecification& specification)
	{
		return CreateRef<VulkanFramebuffer>(specification);
	}

	void VulkanFramebuffer::Release()
	{
		for (uint32_t index = 0; const auto& att : myColorAttachmentImages)
		{
			if (!mySpecification.existingImages.contains(index))
			{
				att->Release();
			}

			index++;
		}

		if (myDepthAttachmentImage && !mySpecification.existingDepth)
		{
			myDepthAttachmentImage->Release();
		}

		myColorFormats.clear();
		myColorAttachmentInfos.clear();
	}
}
