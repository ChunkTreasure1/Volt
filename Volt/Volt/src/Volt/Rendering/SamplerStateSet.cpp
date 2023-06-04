#include "vtpch.h"
#include "SamplerStateSet.h"

#include "Volt/Rendering/VulkanSamplerState.h"

namespace Volt
{
	SamplerStateSet::SamplerStateSet(uint32_t count)
		: myCount(count)
	{}

	SamplerStateSet::~SamplerStateSet()
	{
		mySamplerStates.clear();
	}

	void SamplerStateSet::Add(uint32_t set, uint32_t binding, TextureFilter minFilter, TextureFilter magFilter, TextureFilter mipMode, TextureWrap wrapMode, CompareOperator compareOp, AnisotopyLevel anisoLevel, bool reduce)
	{
		for (uint32_t i = 0; i < myCount; i++)
		{
			mySamplerStates[set][binding].emplace_back(VulkanSamplerState::Create(minFilter, magFilter, mipMode, wrapMode, compareOp, anisoLevel, reduce));
		}
	}

	const std::vector<VkWriteDescriptorSet> SamplerStateSet::GetWriteDescriptors(VkDescriptorSet targetSet, uint32_t index)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptors{};

		for (const auto& [set, bindings] : mySamplerStates)
		{
			for (const auto& [binding, samplerState] : bindings)
			{
				auto& writeDescriptor = writeDescriptors.emplace_back();
				writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptor.descriptorCount = 1;
				writeDescriptor.dstArrayElement = 0;
				writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				writeDescriptor.dstSet = targetSet;
				writeDescriptor.dstBinding = binding;
				writeDescriptor.pImageInfo = &samplerState.at(index)->GetDescriptorInfo();
			}
		}

		return writeDescriptors;
	}

	Ref<SamplerStateSet> SamplerStateSet::Create(uint32_t count)
	{
		return CreateRef<SamplerStateSet>(count);
	}
}
