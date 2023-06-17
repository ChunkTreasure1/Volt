#pragma once

#include "Volt/Rendering/Texture/ImageCommon.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class Image2D;

	struct FramebufferAttachment
	{
		FramebufferAttachment()
		{}

		FramebufferAttachment(ImageFormat aFormat, const gem::vec4 aClearColor = { 1.f, 1.f, 1.f, 1.f }, TextureBlend aBlending = TextureBlend::None, const std::string& aDebugName = "", bool aStorageCompatible = false)
			: format(aFormat), clearColor(aClearColor), debugName(aDebugName), storageCompatible(aStorageCompatible), blending(aBlending)
		{}

		ImageFormat format = ImageFormat::RGBA;
		TextureFilter filter = TextureFilter::Linear;
		TextureWrap wrap = TextureWrap::Repeat;
		TextureBlend blending = TextureBlend::Alpha;
		ClearMode clearMode = ClearMode::Clear;

		gem::vec4 clearColor = { 1.f };
		gem::vec4 borderColor = { 1.f };

		std::string debugName;

		bool storageCompatible = false;

		bool isCubeMap = false;
		uint32_t layers = 1;
	};

	struct VulkanFramebufferSpecification
	{
		uint32_t width = 1280;
		uint32_t height = 720;

		std::vector<FramebufferAttachment> attachments;
		std::map<uint32_t, Ref<Image2D>> existingImages;
		Ref<Image2D> existingDepth;
	};

	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer(const VulkanFramebufferSpecification& specification);
		~VulkanFramebuffer();

		void Invalidate();

		void Bind(VkCommandBuffer commandBuffer) const;
		void Unbind(VkCommandBuffer commandBuffer) const;
		void Resize(uint32_t width, uint32_t height);

		void Clear(VkCommandBuffer commandBuffer);

		inline const Ref<Image2D> GetDepthAttachment() const { return myDepthAttachmentImage; }
		inline const Ref<Image2D> GetColorAttachment(uint32_t index) const { return myColorAttachmentImages.at(index); }
		inline const VulkanFramebufferSpecification& GetSpecification() const { return mySpecification; }

		inline const std::vector<VkRenderingAttachmentInfo>& GetColorAttachmentInfos() const { return myColorAttachmentInfos; }
		inline const VkRenderingAttachmentInfo& GetDepthAttachmentInfo() const { return myDepthAttachmentInfo; }

		inline const std::vector<VkFormat>& GetColorFormats() const { return myColorFormats; }
		inline const VkFormat& GetDepthFormat() const { return myDepthFormat; }

		inline const uint32_t GetWidth() const { return myWidth; }
		inline const uint32_t GetHeight() const { return myHeight; }

		static Ref<VulkanFramebuffer> Create(const VulkanFramebufferSpecification& specification);

	private:
		void Release();

		VulkanFramebufferSpecification mySpecification;
		
		uint32_t myWidth;
		uint32_t myHeight;

		Ref<Image2D> myDepthAttachmentImage;
		std::vector<Ref<Image2D>> myColorAttachmentImages;

		std::vector<VkFormat> myColorFormats;
		VkFormat myDepthFormat;

		std::vector<VkRenderingAttachmentInfo> myColorAttachmentInfos;
		VkRenderingAttachmentInfo myDepthAttachmentInfo;
	};
}
