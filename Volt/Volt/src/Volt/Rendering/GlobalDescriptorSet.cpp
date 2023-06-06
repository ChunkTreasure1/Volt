#include "vtpch.h"
#include "GlobalDescriptorSet.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Graphics/Swapchain.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"


#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	GlobalDescriptorSet::GlobalDescriptorSet(const GlobalDescriptorSetSpecification& specification)
		: mySpecification(specification)
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

		for (const auto& binding : specification.descriptorBindings)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.binding = binding.binding;
			layoutBinding.stageFlags = (VkShaderStageFlagBits)binding.stageFlags;
			layoutBinding.descriptorType = (VkDescriptorType)binding.descriptorType;
			layoutBinding.descriptorCount = binding.descriptorCount;
			layoutBinding.pImmutableSamplers = nullptr;

			if (specification.updateAfterBind)
			{
			}

			myPoolSizes[(VkDescriptorType)binding.descriptorType].count += binding.descriptorCount * 3;
		
			for (uint32_t i = 0; i < mySpecification.descriptorSetCount; i++)
			{
				auto& writeDescriptor = myWriteDescriptors[binding.binding].emplace_back();
				writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptor.pNext = nullptr;
				writeDescriptor.descriptorCount = binding.descriptorCount;
				writeDescriptor.dstArrayElement = 0;
				writeDescriptor.dstBinding = binding.binding;
				writeDescriptor.descriptorType = (VkDescriptorType)binding.descriptorType;
				writeDescriptor.pBufferInfo = nullptr;
				writeDescriptor.pImageInfo = nullptr;
				writeDescriptor.pTexelBufferView = nullptr;
			}
		}

		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.bindingCount = (uint32_t)layoutBindings.size();
		info.pBindings = layoutBindings.data();

		constexpr VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		std::vector<VkDescriptorBindingFlags> bindingFlags{};

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};

		if (specification.updateAfterBind)
		{
			info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

			bindingFlags.resize(layoutBindings.size());
			std::fill(bindingFlags.begin(), bindingFlags.end(), bindlessFlags);

			extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			extendedInfo.bindingCount = (uint32_t)bindingFlags.size();
			extendedInfo.pBindingFlags = bindingFlags.data();

			info.pNext = &extendedInfo;
		}

		auto device = GraphicsContextVolt::GetDevice();
		VT_VK_CHECK(vkCreateDescriptorSetLayout(device->GetHandle(), &info, nullptr, &myDescriptorSetLayout));

		CreateDescriptorPools();
		myDescriptorSets.resize(mySpecification.descriptorSetCount);
	}

	GlobalDescriptorSet::~GlobalDescriptorSet()
	{
		auto device = GraphicsContextVolt::GetDevice();
		vkDestroyDescriptorSetLayout(device->GetHandle(), myDescriptorSetLayout, nullptr);

		for (const auto& pool : myDescriptorPools)
		{
			vkDestroyDescriptorPool(device->GetHandle(), pool, nullptr);
		}
	}

	VkDescriptorSet GlobalDescriptorSet::GetOrAllocateDescriptorSet(uint32_t index)
	{
		std::scoped_lock lock{ myAllocateDescriptorSetMutex };

		if (myDescriptorSets.at(index))
		{
			return myDescriptorSets.at(index);
		}

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = myDescriptorPools.at(index);
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &myDescriptorSetLayout;

		VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};

		if (mySpecification.updateAfterBind)
		{
			countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			countInfo.descriptorSetCount = 1;

			const uint32_t descriptorCount = myMaxTotalDescriptorSets - 1;
			countInfo.pDescriptorCounts = &descriptorCount;
			countInfo.pNext = &countInfo;
		}

		auto device = GraphicsContextVolt::GetDevice();
		VT_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &myDescriptorSets.at(index)));

		for (auto& [binding, writeDescriptors] : myWriteDescriptors)
		{
			writeDescriptors.at(index).dstSet = myDescriptorSets.at(index);
		}

		return myDescriptorSets.at(index);
	}

	VkWriteDescriptorSet GlobalDescriptorSet::GetWriteDescriptor(uint32_t index, uint32_t binding, uint32_t arrayElement)
	{
		return myWriteDescriptors.at(binding).at(index);
	}

	Ref<GlobalDescriptorSet> GlobalDescriptorSet::Create(const GlobalDescriptorSetSpecification& specification)
	{
		return CreateRef<GlobalDescriptorSet>(specification);
	}

	void GlobalDescriptorSet::CreateDescriptorPools()
	{
		std::vector<VkDescriptorPoolSize> poolSizes{};

		for (const auto& [type, count] : myPoolSizes)
		{
			poolSizes.emplace_back(type, count.count);

			myMaxTotalDescriptorSets += count.count;
		}

		VkDescriptorPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

		if (mySpecification.updateAfterBind)
		{
			info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		}
		else
		{
			info.flags = 0;
		}

		info.maxSets = myMaxTotalDescriptorSets;
		info.poolSizeCount = (uint32_t)poolSizes.size();
		info.pPoolSizes = poolSizes.data();

		for (uint32_t i = 0; i < mySpecification.descriptorSetCount; i++)
		{
			VT_VK_CHECK(vkCreateDescriptorPool(GraphicsContextVolt::GetDevice()->GetHandle(), &info, nullptr, &myDescriptorPools.emplace_back()));
		}
	}
}
