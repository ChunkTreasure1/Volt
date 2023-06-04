#pragma once

#include "Volt/Core/Graphics/VulkanAllocator.h"
#include "Volt/Rendering/Texture/ImageCommon.h"

namespace Volt
{
	class Image3D
	{
	public:
		Image3D(const ImageSpecification& specification, const void* data);
		~Image3D();

		void Invalidate(uint32_t width, uint32_t height, uint32_t depth, const void* data = nullptr);
		void Release();

		void TransitionToLayout(VkCommandBuffer commandBuffer, VkImageLayout targetLayout);

		void CreateMipViews();
		VkImageView CreateMipView(const uint32_t mip);

		VkImageAspectFlags GetImageAspect() const;
		void SetName(const std::string& name);
		inline void OverrideLayout(VkImageLayout layout) { myImageData.layout = layout; myDescriptorInfo.imageLayout = layout; }


		inline const VkImage GetHandle() const { return myImage; }
		inline const ImageFormat GetFormat() const { return mySpecification.format; }
		inline const ImageSpecification& GetSpecification() const { return mySpecification; }

		inline const uint32_t GetWidth() const { return mySpecification.width; }
		inline const uint32_t GetHeight() const { return mySpecification.height; }
		inline const uint32_t GetDepth() const { return mySpecification.depth; }

		inline const VkImageView GetView(uint32_t index = 0) const { return myImageViews.at(index); }
		inline const VkSampler GetSampler() const { return myImageData.sampler; }
		inline const VkImageLayout GetLayout() const { return myImageData.layout; }

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return myDescriptorInfo; }

		static Ref<Image3D> Create(const ImageSpecification& specification, const void* data = nullptr);

	private:
		ImageSpecification mySpecification;

		VmaAllocation myAllocation = nullptr;
		VkImage myImage = nullptr;

		struct ImageData
		{
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkSampler sampler = nullptr;
		} myImageData;
		
		std::map<uint32_t, VkImageView> myImageViews;
		VkDescriptorImageInfo myDescriptorInfo{};
	};
}
