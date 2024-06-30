#include "vkpch.h"
#include "VulkanBindlessManager.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/GraphicsContext.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	inline static VkDescriptorSetLayout s_globalDescriptorSetLayout = nullptr;
	inline static VkDescriptorPool s_globalDescriptorPool = nullptr;
	inline static VkDescriptorSet s_globalDescriptorSet = nullptr;

	inline static constexpr uint32_t TEXTURE1D_BINDING = 0;
	inline static constexpr uint32_t TEXTURE2D_BINDING = 1;
	inline static constexpr uint32_t TEXTURE3D_BINDING = 2;
	inline static constexpr uint32_t TEXTURECUBE_BINDING = 3;
	inline static constexpr uint32_t RWTEXTURE1D_BINDING = 4;
	inline static constexpr uint32_t RWTEXTURE2D_BINDING = 5;
	inline static constexpr uint32_t RWTEXTURE3D_BINDING = 6;
	inline static constexpr uint32_t BYTEADDRESSBUFFER_BINDING = 7;
	inline static constexpr uint32_t RWBYTEADDRESSBUFFER_BINDING = 8;
	inline static constexpr uint32_t UNIFORMBUFFER_BINDING = 9;
	inline static constexpr uint32_t SAMPLERSTATE_BINDING = 10;
	inline static constexpr uint32_t RWTEXTURE2DARRAY_BINDING = 11;
	inline static constexpr uint32_t TEXTURE2DARRAY_BINDING = 12;

	void VulkanBindlessManager::CreateGlobalDescriptorLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

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

		std::vector<VkDescriptorBindingFlags> bindingFlags{};

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
		extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		extendedInfo.bindingCount = info.bindingCount;

		constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

		for (const auto& binding : descriptorSetLayoutBindings)
		{
			binding;

			auto& flags = bindingFlags.emplace_back();
			flags = 0;

			flags = bindlessFlags;
			if (!usingDescriptorBuffers)
			{
				info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
				flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			}
		}

		extendedInfo.pBindingFlags = bindingFlags.data();
		info.pNext = &extendedInfo;

		VT_VK_CHECK(vkCreateDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &info, nullptr, &s_globalDescriptorSetLayout));

		constexpr VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10000 },
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.maxSets = 100'000;
		poolInfo.poolSizeCount = 5;
		poolInfo.pPoolSizes = poolSizes;

		VT_VK_CHECK(vkCreateDescriptorPool(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &poolInfo, nullptr, &s_globalDescriptorPool));

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = s_globalDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &s_globalDescriptorSetLayout;

		VT_VK_CHECK(vkAllocateDescriptorSets(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &allocInfo, &s_globalDescriptorSet));
	}

	void VulkanBindlessManager::DestroyGlobalDescriptorLayout()
	{
		vkDestroyDescriptorSetLayout(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), s_globalDescriptorSetLayout, nullptr);
		s_globalDescriptorSetLayout = nullptr;
	}

	VkDescriptorSetLayout_T* VulkanBindlessManager::GetGlobalDescriptorSetLayout()
	{
		return s_globalDescriptorSetLayout;
	}

	VkDescriptorSet_T* VulkanBindlessManager::GetGlobalDescriptorSet()
	{
		return s_globalDescriptorSet;
	}
}
