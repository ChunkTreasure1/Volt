#include "vkpch.h"
#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorLayoutManager.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Globals.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	inline static VkDescriptorSetLayout s_globalDescriptorSetLayout = nullptr;
	inline static VkDescriptorSetLayout s_renderGraphConstantsLayout = nullptr;

	void VulkanBindlessDescriptorLayoutManager::CreateGlobalDescriptorLayout()
	{
		VT_ENSURE(TryCreateMutableDescriptorSetLayout(s_globalDescriptorSetLayout));

		// Setup render graph constants descriptor set layout
		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = Globals::RENDER_GRAPH_CONSTANTS_BINDING;
			binding.descriptorCount = 1;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			binding.pImmutableSamplers = nullptr;
			binding.stageFlags = VK_SHADER_STAGE_ALL;

			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.bindingCount = 1;
			info.pBindings = &binding;
			info.flags = 0;

			VT_VK_CHECK(vkCreateDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &info, nullptr, &s_renderGraphConstantsLayout));
		}
	}

	void VulkanBindlessDescriptorLayoutManager::DestroyGlobalDescriptorLayout()
	{
		vkDestroyDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), s_globalDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), s_renderGraphConstantsLayout, nullptr);
		s_globalDescriptorSetLayout = nullptr;
		s_renderGraphConstantsLayout = nullptr;
	}

	std::array<VkDescriptorSetLayout_T*, 2> VulkanBindlessDescriptorLayoutManager::GetGlobalDescriptorSetLayouts()
	{
		return { s_globalDescriptorSetLayout, s_renderGraphConstantsLayout };
	}

	bool VulkanBindlessDescriptorLayoutManager::TryCreateMutableDescriptorSetLayout(VkDescriptorSetLayout_T*& outDescriptorSetLayouts)
	{
		constexpr uint32_t DescriptorTypeCount = 7;

		std::array<VkDescriptorType, DescriptorTypeCount> heapDescriptorTypes =
		{
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
			VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		};

		Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

		VkMutableDescriptorTypeListEXT descriptorTypeList{};
		descriptorTypeList.descriptorTypeCount = DescriptorTypeCount;
		descriptorTypeList.pDescriptorTypes = heapDescriptorTypes.data();

		VkMutableDescriptorTypeCreateInfoEXT mutableDescriptorInfo{};
		mutableDescriptorInfo.sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT;
		mutableDescriptorInfo.pNext = nullptr;
		mutableDescriptorInfo.mutableDescriptorTypeListCount = 1;
		mutableDescriptorInfo.pMutableDescriptorTypeLists = &descriptorTypeList;

		{
			VkDescriptorSetLayoutBinding& cbvSrvUavBinding = descriptorSetLayoutBindings.emplace_back();
			cbvSrvUavBinding.binding = CBV_SRV_UAV_BINDING;
			cbvSrvUavBinding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
			cbvSrvUavBinding.descriptorCount = std::numeric_limits<uint16_t>::max();
			cbvSrvUavBinding.pImmutableSamplers = nullptr;
			cbvSrvUavBinding.stageFlags = VK_SHADER_STAGE_ALL;
		}

		{
			VkDescriptorSetLayoutBinding& samplersBinding = descriptorSetLayoutBindings.emplace_back();
			samplersBinding.binding = SAMPLERS_BINDING;
			samplersBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			samplersBinding.descriptorCount = std::numeric_limits<uint16_t>::max();
			samplersBinding.pImmutableSamplers = nullptr;
			samplersBinding.stageFlags = VK_SHADER_STAGE_ALL;
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
		createInfo.pBindings = descriptorSetLayoutBindings.data();
		createInfo.flags = 0;

		Vector<VkDescriptorBindingFlags> bindingFlags{};

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
		extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		extendedInfo.pNext = &mutableDescriptorInfo;
		extendedInfo.bindingCount = createInfo.bindingCount;

		constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

		for (const auto& binding : descriptorSetLayoutBindings)
		{
			VT_UNUSED(binding);

			auto& flags = bindingFlags.emplace_back();
			flags = 0;

			flags = bindlessFlags;
			if (binding.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC && binding.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
			{
				createInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
				flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			}
		}

		extendedInfo.pBindingFlags = bindingFlags.data();
		createInfo.pNext = &extendedInfo;

		VkDescriptorSetLayoutSupport support{};
		support.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT;
		support.pNext = nullptr;

		vkGetDescriptorSetLayoutSupport(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &createInfo, &support);

		if (!support.supported)
		{
			return false;
		}

		VT_VK_CHECK(vkCreateDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &createInfo, nullptr, &outDescriptorSetLayouts));
		return true;
	}
}
