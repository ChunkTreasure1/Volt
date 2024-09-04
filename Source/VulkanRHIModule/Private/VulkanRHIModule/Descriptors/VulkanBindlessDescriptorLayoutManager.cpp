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
		// Setup main descriptor set layout
		{
			Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = TEXTURE1D_BINDING;
				binding.descriptorCount = VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = TEXTURE2D_BINDING;
				binding.descriptorCount = VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = TEXTURE2DARRAY_BINDING;
				binding.descriptorCount = VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = TEXTURE3D_BINDING;
				binding.descriptorCount = VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = TEXTURECUBE_BINDING;
				binding.descriptorCount = VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = RWTEXTURE1D_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = RWTEXTURE2D_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = RWTEXTURE2DARRAY_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = RWTEXTURE3D_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_IMAGE_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = BYTEADDRESSBUFFER_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_BUFFER_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = RWBYTEADDRESSBUFFER_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_BUFFER_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = UNIFORMBUFFER_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_BUFFER_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			{
				auto& binding = descriptorSetLayoutBindings.emplace_back();
				binding.binding = SAMPLERSTATE_BINDING;
				binding.descriptorCount = VulkanDefaults::STORAGE_BUFFER_BINDLESS_TABLE_SIZE;
				binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				binding.stageFlags = VK_SHADER_STAGE_ALL;
			}

			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
			info.pBindings = descriptorSetLayoutBindings.data();
			info.flags = 0;

			const bool usingDescriptorBuffers = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled();

			if (usingDescriptorBuffers)
			{
				info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
			}

			Vector<VkDescriptorBindingFlags> bindingFlags{};

			VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
			extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			extendedInfo.bindingCount = info.bindingCount;

			constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

			for (const auto& binding : descriptorSetLayoutBindings)
			{
				VT_UNUSED(binding);

				auto& flags = bindingFlags.emplace_back();
				flags = 0;

				flags = bindlessFlags;
				if (!usingDescriptorBuffers && binding.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC && binding.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
				{
					info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
					flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
				}
			}

			extendedInfo.pBindingFlags = bindingFlags.data();
			info.pNext = &extendedInfo;

			VT_VK_CHECK(vkCreateDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &info, nullptr, &s_globalDescriptorSetLayout));
		}

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
}
