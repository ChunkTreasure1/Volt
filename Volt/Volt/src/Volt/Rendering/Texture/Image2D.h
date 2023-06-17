#pragma once

#include "Volt/Rendering/Texture/ImageCommon.h"

#include "Volt/Core/Graphics/VulkanAllocator.h"
#include "Volt/Core/Buffer.h"

#include <vulkan/vulkan.h>
#include <map>

namespace Volt
{
	class CommandBuffer;
	class Image2D
	{
	public:
		Image2D(const ImageSpecification& specification, const void* data);
		Image2D(const ImageSpecification& specification, bool transitionTolayout);
		~Image2D();

		void Invalidate(uint32_t width, uint32_t height, bool transitionLayout = true, const void* data = nullptr);
		void Release();

		void TransitionToLayout(VkCommandBuffer commandBuffer, VkImageLayout targetLayout);
		void GenerateMips(bool readOnly = false, VkCommandBuffer commandBuffer = nullptr); // Optional command buffer, otherwise it will create it's own
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
		const uint32_t CalculateMipCount() const;

		inline const VkImageView GetView(uint32_t index = 0) const { return myImageViews.at(index); }
		inline const VkImageView GetArrayView(uint32_t index = 0) const { return myArrayImageViews.at(index); } // Only applicable for Cubic images
		inline const VkSampler GetSampler() const { return myImageData.sampler; }
		inline const VkImageLayout GetLayout() const { return myImageData.layout; }

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return myDescriptorInfo; }
		const VkDescriptorImageInfo& GetDescriptorInfo(VkImageLayout targetLayout);

		void CopyFromImage(VkCommandBuffer commandBuffer, Ref<Image2D> srcImage);

		template<typename T>
		inline T ReadPixel(uint32_t x, uint32_t y);

		template<typename T>
		inline std::vector<T> ReadPixelRange(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY);

		template<typename T>
		inline T* Map();

		void Unmap();

		static Ref<Image2D> Create(const ImageSpecification& specification, const void* data = nullptr);
		static Ref<Image2D> Create(const ImageSpecification& specification, bool transitionTolayout);

	private:
		friend class TransientResourceSystem;

		Buffer ReadPixelInternal(uint32_t x, uint32_t y, uint32_t size);
		Buffer ReadPixelRangeInternal(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY, uint32_t size);
		void* MapInternal();

		ImageSpecification mySpecification;

		VmaAllocation myAllocation = nullptr;
		VkImage myImage = nullptr;

		VmaAllocation myMappingAllocation = nullptr;
		VkBuffer myMappingBuffer = nullptr;

		struct ImageData
		{
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkSampler sampler = nullptr;
		} myImageData;

		std::map<uint32_t, VkImageView> myImageViews;
		std::map<uint32_t, VkImageView> myArrayImageViews;

		VkDescriptorImageInfo myDescriptorInfo{};
		bool myHasGeneratedMips = false;
		bool myAllocatedWithCustomPool = false;
	};

	template<typename T>
	inline T Image2D::ReadPixel(uint32_t x, uint32_t y)
	{
		auto buffer = ReadPixelInternal(x, y, sizeof(T));
		T value = *buffer.As<T>();
		buffer.Release();

		return value;
	}

	template<typename T>
	inline std::vector<T> Image2D::ReadPixelRange(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY)
	{
		auto buffer = ReadPixelRangeInternal(minX, minY, maxX, maxY, sizeof(T));
		
		std::vector<T> result{};
		result.resize(buffer.GetSize() / sizeof(T));
		memcpy_s(result.data(), result.size() * sizeof(T), buffer.As<void>(), buffer.GetSize());
		buffer.Release();

		return result;
	}

	template<typename T>
	inline T* Image2D::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}
}
