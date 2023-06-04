#pragma once

#include "Volt/Rendering/Texture/ImageCommon.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class VulkanSamplerState;
	class SamplerStateSet
	{
	public:
		SamplerStateSet(uint32_t count);
		~SamplerStateSet();

		void Add(uint32_t set, uint32_t binding, TextureFilter minFilter, TextureFilter magFilter, TextureFilter mipMode, TextureWrap wrapMode, CompareOperator compareOp, AnisotopyLevel anisoLevel = AnisotopyLevel::None, bool reduce = false);
		
		const std::vector<VkWriteDescriptorSet> GetWriteDescriptors(VkDescriptorSet targetSet, uint32_t index);
		
		static Ref<SamplerStateSet> Create(uint32_t count);
	
	private:
		uint32_t myCount = 0;
		std::map<uint32_t, std::map<uint32_t, std::vector<Ref<VulkanSamplerState>>>> mySamplerStates;
	};
}
