#pragma once

#include "Volt/Rendering/FrameGraph/FrameGraphResourceHandle.h"
#include "Volt/Rendering/Texture/ImageCommon.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Volt
{
	struct FrameGraphRenderPassNodeBase;
	class FrameGraph;
	class Image2D;
	class Image3D;

	struct FrameGraphTextureSpecification
	{
		FrameGraphTextureSpecification()
		{}

		FrameGraphTextureSpecification(ImageFormat aFormat, const glm::uvec2& size, const glm::vec4 aClearColor = { 1.f, 1.f, 1.f, 1.f }, const std::string& aName = "", ImageUsage aUsage = ImageUsage::Attachment)
			: format(aFormat), clearColor(aClearColor), name(aName), usage(aUsage), width(size.x), height(size.y)
		{}

		FrameGraphTextureSpecification(ImageFormat aFormat, const glm::uvec2& size, const std::string& aName, ImageUsage aUsage = ImageUsage::Attachment)
			: format(aFormat), name(aName), usage(aUsage), width(size.x), height(size.y)
		{}

		FrameGraphTextureSpecification(const std::string& aName, ClearMode aClearMode = ClearMode::Clear)
			: name(aName), clearMode(aClearMode)
		{}

		ImageFormat format = ImageFormat::RGBA;
		ImageUsage usage = ImageUsage::Attachment;
		ClearMode clearMode = ClearMode::Clear;

		glm::vec4 clearColor = { 0.f };

		std::string name;

		uint32_t width = 1;
		uint32_t height = 1;

		bool isCubeMap = false;
		uint32_t layers = 1;
		uint32_t mips = 1;
	};

	struct FrameGraphTexture
	{
		FrameGraphTextureSpecification specification;
		Weak<Image2D> image;
		Weak<Image3D> image3D;
		bool isExternal = false;
	};

	struct FrameGraphResourceAccess
	{
		VkPipelineStageFlagBits2 dstStage;
		VkAccessFlagBits2 dstAccess;
		VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	struct FrameGraphResourceNode
	{
		uint32_t refCount = 0;
		Weak<FrameGraphRenderPassNodeBase> producer;
		Weak<FrameGraphRenderPassNodeBase> lastUsage;

		FrameGraphResourceHandle handle;
		FrameGraphTexture resource;
	};

	class FrameGraphRenderPassResources
	{
	public:
		FrameGraphRenderPassResources(FrameGraph& frameGraph, FrameGraphRenderPassNodeBase& renderPass);

		const FrameGraphTexture& GetImageResource(FrameGraphResourceHandle handle);

	private:
		FrameGraph& myFrameGraph;
		FrameGraphRenderPassNodeBase& myRenderPass;
	};
}
