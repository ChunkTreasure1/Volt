#include "vkpch.h"
#include "VulkanSamplerState.h"

#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanSamplerState::VulkanSamplerState(const SamplerStateCreateInfo& createInfo)
	{
		VkSamplerCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.anisotropyEnable = createInfo.anisotropyLevel != AnisotropyLevel::None ? true : false;
		info.maxAnisotropy = static_cast<float>(createInfo.anisotropyLevel);

		info.magFilter = Utility::VoltToVulkanFilter(createInfo.magFilter);
		info.minFilter = Utility::VoltToVulkanFilter(createInfo.minFilter);
		info.mipmapMode = Utility::VoltToVulkanMipMapMode(createInfo.mipFilter);

		info.addressModeU = Utility::VoltToVulkanWrapMode(createInfo.wrapMode);
		info.addressModeV = Utility::VoltToVulkanWrapMode(createInfo.wrapMode);
		info.addressModeW = Utility::VoltToVulkanWrapMode(createInfo.wrapMode);

		info.mipLodBias = createInfo.mipLodBias;
		info.minLod = createInfo.minLod;
		info.maxLod = createInfo.maxLod;

		info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		info.compareEnable = createInfo.compareOperator != CompareOperator::None ? true : false;
		info.compareOp = createInfo.compareOperator == CompareOperator::None ? VK_COMPARE_OP_ALWAYS : Utility::VoltToVulkanCompareOp(createInfo.compareOperator);

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateSampler(device->GetHandle<VkDevice>(), &info, nullptr, &m_sampler));
	}

	VulkanSamplerState::~VulkanSamplerState()
	{
		GraphicsContext::DestroyResource([sampler = m_sampler]() 
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroySampler(device->GetHandle<VkDevice>(), sampler, nullptr);
		});

		m_sampler = nullptr;
	}

	void* VulkanSamplerState::GetHandleImpl()
	{
		return m_sampler;
	}
}
