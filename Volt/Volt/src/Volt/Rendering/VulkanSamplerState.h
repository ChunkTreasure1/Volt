#pragma once

#include "Volt/Rendering/Texture/ImageCommon.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class VulkanSamplerState
	{
	public:
		VulkanSamplerState(TextureFilter minFilter, TextureFilter magFilter, TextureFilter mipMode, TextureWrap wrapMode, CompareOperator compareOp, AnisotopyLevel anisoLevel, bool reduction);
		~VulkanSamplerState();

		inline VkSampler GetHandle() const { return mySamplerState; }
		static Ref<VulkanSamplerState> Create(TextureFilter minFilter, TextureFilter magFilter, TextureFilter mipMode, TextureWrap wrapMode, CompareOperator compareOp, AnisotopyLevel anisoLevel = AnisotopyLevel::None, bool reduction = false);

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return myDescriptorInfo; }

	private:
		VkDescriptorImageInfo myDescriptorInfo{};
		VkSampler mySamplerState = nullptr;
	};
}
