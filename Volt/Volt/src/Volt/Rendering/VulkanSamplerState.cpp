#include "vtpch.h"
#include "VulkanSamplerState.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	VulkanSamplerState::VulkanSamplerState(TextureFilter minFilter, TextureFilter magFilter, TextureFilter mipMode, TextureWrap wrapMode, CompareOperator compareOp, AnisotopyLevel anisoLevel, bool reduction)
	{
		VkSamplerCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.anisotropyEnable = anisoLevel != AnisotopyLevel::None ? true : false;
		info.maxAnisotropy = (float)anisoLevel;
		
		info.magFilter = Utility::VoltToVulkanFilter(magFilter);
		info.minFilter = Utility::VoltToVulkanFilter(minFilter);
		info.mipmapMode = Utility::VolToVulkanMipMapMode(mipMode);
	
		info.addressModeU = Utility::VoltToVulkanWrapMode(wrapMode);
		info.addressModeV = Utility::VoltToVulkanWrapMode(wrapMode);
		info.addressModeW = Utility::VoltToVulkanWrapMode(wrapMode);
	
		info.mipLodBias = 0.f;
		info.minLod = 0.f;
		info.maxLod = FLT_MAX;

		info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		info.compareEnable = compareOp != CompareOperator::None ? true : false;
		info.compareOp = compareOp == CompareOperator::None ? VK_COMPARE_OP_ALWAYS : Utility::VoltToVulkanCompareOp(compareOp);
	
		VkSamplerReductionModeCreateInfo reductionInfo{};
		reductionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
		reductionInfo.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
		
		if (reduction)
		{
			info.pNext = &reductionInfo;
		}

		//auto device = GraphicsContextVolt::GetDevice();
		//VT_VK_CHECK(vkCreateSampler(device->GetHandle(), &info, nullptr, &mySamplerState));

		myDescriptorInfo.sampler = mySamplerState;
		myDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		myDescriptorInfo.imageView = nullptr;
	}

	VulkanSamplerState::~VulkanSamplerState()
	{
		//auto device = GraphicsContextVolt::GetDevice();
		//vkDestroySampler(device->GetHandle(), mySamplerState, nullptr);
	}

	Ref<VulkanSamplerState> VulkanSamplerState::Create(TextureFilter minFilter, TextureFilter magFilter, TextureFilter mipMode, TextureWrap wrapMode, CompareOperator compareOp, AnisotopyLevel anisoLevel, bool reduce)
	{
		return CreateRef<VulkanSamplerState>(minFilter, magFilter, mipMode, wrapMode, compareOp, anisoLevel, reduce);
	}
}
