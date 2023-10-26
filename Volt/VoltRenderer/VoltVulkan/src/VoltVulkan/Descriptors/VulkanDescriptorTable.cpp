#include "vkpch.h"
#include "VulkanDescriptorTable.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"
#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Images/VulkanImageView.h"
#include "VoltVulkan/Buffers/VulkanBufferView.h"
#include "VoltVulkan/Buffers/VulkanStorageBuffer.h"
#include "VoltVulkan/Buffers/VulkanCommandBuffer.h"
#include "VoltVulkan/Descriptors/VulkanBindlessManager.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Buffers/BufferViewSet.h>

#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Core/Profiling.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline VkImageLayout GetImageLayoutFromDescriptorType(VkDescriptorType descriptorType)
		{
			switch (descriptorType)
			{
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return VK_IMAGE_LAYOUT_GENERAL;
			}
			
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VulkanDescriptorTable::VulkanDescriptorTable(const DescriptorTableCreateInfo& specification)
	{
		m_shader = specification.shader;
		m_descriptorPoolCount = specification.count;

		Invalidate();
	}

	VulkanDescriptorTable::~VulkanDescriptorTable()
	{
		Release();
	}

	void VulkanDescriptorTable::SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			return;
		}

		SetDirty(true);
		m_imageInfos[set][binding][arrayIndex].resize(m_descriptorPoolCount);

		for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
		{
			auto& description = m_imageInfos[set][binding][arrayIndex].at(i);
			description.imageView = imageView->GetHandle<VkImageView>();
			description.sampler = nullptr;

			uint32_t writeDescriptorIndex = 0;

			// Mapping for write descriptor does not exist
			if (m_activeWriteDescriptorsMapping[set][binding][arrayIndex].size() <= static_cast<size_t>(i))
			{
				writeDescriptorIndex = m_writeDescriptorsMapping[set][binding][i];
				WriteDescriptor writeDescriptorCopy = m_writeDescriptors.at(i).at(writeDescriptorIndex);
				writeDescriptorCopy.dstArrayElement = arrayIndex;
				writeDescriptorCopy.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);

				m_activeWriteDescriptors.at(i).emplace_back(writeDescriptorCopy);

				writeDescriptorIndex = static_cast<uint32_t>(m_activeWriteDescriptors.at(i).size() - 1);
				m_activeWriteDescriptorsMapping[set][binding][arrayIndex].emplace_back() = writeDescriptorIndex;
			}
			else
			{
				writeDescriptorIndex = m_activeWriteDescriptorsMapping[set][binding][arrayIndex][i];
				m_activeWriteDescriptors.at(i).at(writeDescriptorIndex).pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);
			}

			description.imageLayout = Utility::GetImageLayoutFromDescriptorType(static_cast<VkDescriptorType>(m_activeWriteDescriptors.at(i).at(writeDescriptorIndex).descriptorType));
		}
	}

	void VulkanDescriptorTable::SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			return;
		}

		SetDirty(true);

		VulkanBufferView& vkBufferView = bufferView->AsRef<VulkanBufferView>();
		auto resource = vkBufferView.GetResource();

		m_bufferInfos[set][binding][arrayIndex].resize(m_descriptorPoolCount);
		const auto type = resource->GetType();

		for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
		{
			auto& description = m_bufferInfos[set][binding][arrayIndex].at(i);
			description.buffer = bufferView->GetHandle<VkBuffer>();

			if (type == ResourceType::StorageBuffer)
			{
				description.range = resource->AsRef<VulkanStorageBuffer>().GetByteSize();
			}

			// Mapping for write descriptor does not exist
			if (m_activeWriteDescriptorsMapping[set][binding][arrayIndex].size() <= static_cast<size_t>(i))
			{
				const uint32_t writeDescriptorIndex = m_writeDescriptorsMapping[set][binding][i];
				WriteDescriptor writeDescriptorCopy = m_writeDescriptors.at(i).at(writeDescriptorIndex);
				writeDescriptorCopy.dstArrayElement = arrayIndex;
				writeDescriptorCopy.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&description);

 				m_activeWriteDescriptors.at(i).emplace_back(writeDescriptorCopy);
				m_activeWriteDescriptorsMapping[set][binding][arrayIndex].emplace_back() = static_cast<uint32_t>(m_activeWriteDescriptors.at(i).size() - 1);
			}
			else
			{
				const uint32_t writeDescriptorIndex = m_activeWriteDescriptorsMapping[set][binding][arrayIndex][i];
				m_activeWriteDescriptors.at(i).at(writeDescriptorIndex).pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&description);
			}
		}
	}

	void VulkanDescriptorTable::SetBufferViewSet(Ref<BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			return;
		}

		SetDirty(true);

		const auto& views = bufferViewSet->GetViews();

		VulkanBufferView& vkBufferView = views.front()->AsRef<VulkanBufferView>();
		auto resource = vkBufferView.GetResource();

		m_bufferInfos[set][binding][arrayIndex].resize(m_descriptorPoolCount);
		const auto type = resource->GetType();

		for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
		{
			auto& description = m_bufferInfos[set][binding][arrayIndex].at(i);
			description.buffer = views.at(i)->GetHandle<VkBuffer>();

			if (type == ResourceType::StorageBuffer)
			{
				description.range = resource->AsRef<VulkanStorageBuffer>().GetByteSize();
			}

			// Mapping for write descriptor does not exist
			if (m_activeWriteDescriptorsMapping[set][binding][arrayIndex].size() <= static_cast<size_t>(i))
			{
				const uint32_t writeDescriptorIndex = m_writeDescriptorsMapping[set][binding][i];
				WriteDescriptor writeDescriptorCopy = m_writeDescriptors.at(i).at(writeDescriptorIndex);
				writeDescriptorCopy.dstArrayElement = arrayIndex;
				writeDescriptorCopy.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&description);

				m_activeWriteDescriptors.at(i).emplace_back(writeDescriptorCopy);
				m_activeWriteDescriptorsMapping[set][binding][arrayIndex].emplace_back() = static_cast<uint32_t>(m_activeWriteDescriptors.at(i).size() - 1);
			}
			else
			{
				const uint32_t writeDescriptorIndex = m_activeWriteDescriptorsMapping[set][binding][arrayIndex][i];
				m_activeWriteDescriptors.at(i).at(writeDescriptorIndex).pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&description);
			}
		}
	}

	void VulkanDescriptorTable::SetImageView(std::string_view name, Ref<ImageView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetImageView(view, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorTable::SetBufferView(std::string_view name, Ref<BufferView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetBufferView(view, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorTable::SetBufferViews(const std::vector<Ref<BufferView>>& bufferViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset)
	{
		for (uint32_t index = arrayStartOffset; const auto& view : bufferViews)
		{
			SetBufferView(view, set, binding, index);
			index++;
		}
	}

	void VulkanDescriptorTable::SetImageViews(const std::vector<Ref<ImageView>>& imageViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset)
	{
		for (uint32_t index = arrayStartOffset; const auto & view : imageViews)
		{
			SetImageView(view, set, binding, index);
			index++;
		}
	}

	void VulkanDescriptorTable::SetSamplerState(Ref<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			return;
		}

		SetDirty(true);
		m_imageInfos[set][binding][arrayIndex].resize(m_descriptorPoolCount);

		for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
		{
			auto& description = m_imageInfos[set][binding][arrayIndex].at(i);
			description.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			description.imageView = nullptr;
			description.sampler = samplerState->GetHandle<VkSampler>();

			// Mapping for write descriptor does not exist
			if (m_activeWriteDescriptorsMapping[set][binding][arrayIndex].size() <= static_cast<size_t>(i))
			{
				const uint32_t writeDescriptorIndex = m_writeDescriptorsMapping[set][binding][i];
				WriteDescriptor writeDescriptorCopy = m_writeDescriptors.at(i).at(writeDescriptorIndex);
				writeDescriptorCopy.dstArrayElement = arrayIndex;
				writeDescriptorCopy.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);

				m_activeWriteDescriptors.at(i).emplace_back(writeDescriptorCopy);
				m_activeWriteDescriptorsMapping[set][binding][arrayIndex].emplace_back() = static_cast<uint32_t>(m_activeWriteDescriptors.at(i).size() - 1);
			}
			else
			{
				const uint32_t writeDescriptorIndex = m_activeWriteDescriptorsMapping[set][binding][arrayIndex][i];
				m_activeWriteDescriptors.at(i).at(writeDescriptorIndex).pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);
			}
		}
	}

	void VulkanDescriptorTable::Update(const uint32_t index)
	{
		uint32_t actualIndex = index;

		if (m_descriptorPoolCount == 1)
		{
			actualIndex = 0;
		}

		if (!m_isDirty[actualIndex])
		{
			return;
		}

		m_isDirty[actualIndex] = false;

		if (m_activeWriteDescriptors.empty())
		{
			return;
		}

		if (m_activeWriteDescriptors.at(actualIndex).empty())
		{
			return;
		}

		auto& writeDescriptors = m_activeWriteDescriptors.at(actualIndex);

		auto device = GraphicsContext::GetDevice();
		const VkWriteDescriptorSet* writeDescriptorsPtr = reinterpret_cast<const VkWriteDescriptorSet*>(writeDescriptors.data());

		vkUpdateDescriptorSets(device->GetHandle<VkDevice>(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptorsPtr, 0, nullptr);

		writeDescriptors.clear();
		m_activeWriteDescriptorsMapping.clear();
	}

	void* VulkanDescriptorTable::GetHandleImpl() const
	{
		return nullptr;
	}

	void VulkanDescriptorTable::Bind(Ref<CommandBuffer> commandBuffer)
	{
		VulkanCommandBuffer& vulkanCommandBuffer = commandBuffer->AsRef<VulkanCommandBuffer>();

		const VkPipelineBindPoint bindPoint = vulkanCommandBuffer.m_currentRenderPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
		const auto descriptorSet = VulkanBindlessManager::GetGlobalDescriptorSet();
		vkCmdBindDescriptorSets(vulkanCommandBuffer.GetHandle<VkCommandBuffer>(), bindPoint, vulkanCommandBuffer.GetCurrentPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

		// #TODO_Ivar: move to an implementation that binds all descriptor sets in one call
		//for (const auto& [set, sets] : GetDescriptorSets())
		//{
		//	const bool isSingleFrameSet = sets.size() == 1;
		//	vkCmdBindDescriptorSets(vulkanCommandBuffer.GetHandle<VkCommandBuffer>(), bindPoint, vulkanCommandBuffer.GetCurrentPipelineLayout(), set, 1, &sets.at(isSingleFrameSet ? 0 : index), 0, nullptr);
		//}
	}

	void VulkanDescriptorTable::SetDirty(bool state)
	{
		for (auto dirty : m_isDirty)
		{
			dirty = state;
		}
	}

	void VulkanDescriptorTable::Invalidate()
	{
		Release();

		VT_PROFILE_FUNCTION();

		m_isDirty = std::vector<bool>(3, true);
		m_maxTotalDescriptorCount = 0;
		m_imageInfos.clear();
		m_bufferInfos.clear();

		Ref<VulkanShader> vulkanShader = m_shader->As<VulkanShader>();
		const auto& shaderPoolSizes = vulkanShader->GetDescriptorPoolSizes();

		std::vector<VkDescriptorPoolSize> poolSizes{};
		for (const auto& [type, count] : shaderPoolSizes)
		{
			poolSizes.emplace_back(static_cast<VkDescriptorType>(type), count);
			m_maxTotalDescriptorCount += count;
		}

		//if (m_maxTotalDescriptorCount == 0)
		//{
		//	return;
		//}

		VkDescriptorPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		info.maxSets = m_maxTotalDescriptorCount;
		info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		info.pPoolSizes = poolSizes.data();

		auto device = GraphicsContext::GetDevice();

		for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
		{
			VT_PROFILE_SCOPE("Create Descriptor Pool");
			VT_VK_CHECK(vkCreateDescriptorPool(device->GetHandle<VkDevice>(), &info, nullptr, &m_descriptorPools.emplace_back()));
		}

		const auto& usedSets = vulkanShader->GetResources().usedSets;

		// Allocate descriptor sets
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorSetCount = 1;

		for (const auto& set : usedSets)
		{
			m_descriptorSets[set].resize(m_descriptorPoolCount);

			for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
			{
				VT_PROFILE_SCOPE("Allocate DescriptorSet");

				allocInfo.descriptorPool = m_descriptorPools.at(i);
				allocInfo.pSetLayouts = &vulkanShader->GetPaddedDescriptorSetLayouts().at(set);

				VT_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle<VkDevice>(), &allocInfo, &m_descriptorSets.at(set)[i]));
			}
		}

		m_descriptorSets[0][0] = VulkanBindlessManager::GetGlobalDescriptorSet();

		BuildWriteDescriptors();
		InitializeInfoStructs();
	}

	void VulkanDescriptorTable::Release()
	{
		VT_PROFILE_FUNCTION();

		if (m_descriptorPools.empty())
		{
			return;
		}

		GraphicsContext::DestroyResource([descriptorPools = m_descriptorPools]() 
		{
			auto device = GraphicsContext::GetDevice();

			for (const auto& pool : descriptorPools)
			{
				vkDestroyDescriptorPool(device->GetHandle<VkDevice>(), pool, nullptr);
			}
		});

		m_descriptorPools.clear();
	}

	void VulkanDescriptorTable::BuildWriteDescriptors()
	{
		VT_PROFILE_FUNCTION();

		const auto& resources = m_shader->GetResources();

		for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
		{
			m_writeDescriptors.emplace_back();
			m_activeWriteDescriptors.emplace_back();

			for (const auto& [set, bindings] : resources.uniformBuffers)
			{
				for (const auto& [binding, data] : bindings)
				{
					auto& writeDescriptor = m_writeDescriptors[i].emplace_back();
					writeDescriptor.sType = static_cast<VkStructureType>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
					writeDescriptor.pNext = nullptr;
					writeDescriptor.descriptorCount = 1;
					writeDescriptor.dstArrayElement = 0;
					writeDescriptor.dstBinding = binding;
					writeDescriptor.descriptorType = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
					writeDescriptor.dstSet = m_descriptorSets[set][i];

					m_writeDescriptorsMapping[set][binding].emplace_back() = static_cast<uint32_t>(m_writeDescriptors.at(i).size() - 1);
				}
			}

			for (const auto& [set, bindings] : resources.storageBuffers)
			{
				for (const auto& [binding, data] : bindings)
				{
					auto& writeDescriptor = m_writeDescriptors[i].emplace_back();
					writeDescriptor.sType = static_cast<VkStructureType>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
					writeDescriptor.pNext = nullptr;
					writeDescriptor.descriptorCount = 1;
					writeDescriptor.dstArrayElement = 0;
					writeDescriptor.dstBinding = binding;
					writeDescriptor.descriptorType = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
					writeDescriptor.dstSet = m_descriptorSets[set][i];

					m_writeDescriptorsMapping[set][binding].emplace_back() = static_cast<uint32_t>(m_writeDescriptors.at(i).size() - 1);
				}
			}

			for (const auto& [set, bindings] : resources.storageImages)
			{
				for (const auto& [binding, data] : bindings)
				{
					auto& writeDescriptor = m_writeDescriptors[i].emplace_back();
					writeDescriptor.sType = static_cast<VkStructureType>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
					writeDescriptor.pNext = nullptr;
					writeDescriptor.descriptorCount = 1;
					writeDescriptor.dstArrayElement = 0;
					writeDescriptor.dstBinding = binding;
					writeDescriptor.descriptorType = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
					writeDescriptor.dstSet = m_descriptorSets[set][i];

					m_writeDescriptorsMapping[set][binding].emplace_back() = static_cast<uint32_t>(m_writeDescriptors.at(i).size() - 1);
				}
			}

			for (const auto& [set, bindings] : resources.images)
			{
				for (const auto& [binding, data] : bindings)
				{
					auto& writeDescriptor = m_writeDescriptors[i].emplace_back();
					writeDescriptor.sType = static_cast<VkStructureType>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
					writeDescriptor.pNext = nullptr;
					writeDescriptor.descriptorCount = 1;
					writeDescriptor.dstArrayElement = 0;
					writeDescriptor.dstBinding = binding;
					writeDescriptor.descriptorType = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
					writeDescriptor.dstSet = m_descriptorSets[set][i];

					m_writeDescriptorsMapping[set][binding].emplace_back() = static_cast<uint32_t>(m_writeDescriptors.at(i).size() - 1);
				}
			}

			for (const auto& [set, bindings] : resources.samplers)
			{
				for (const auto& [binding, data] : bindings)
				{
					auto& writeDescriptor = m_writeDescriptors[i].emplace_back();
					writeDescriptor.sType = static_cast<VkStructureType>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
					writeDescriptor.pNext = nullptr;
					writeDescriptor.descriptorCount = 1;
					writeDescriptor.dstArrayElement = 0;
					writeDescriptor.dstBinding = binding;
					writeDescriptor.descriptorType = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLER);
					writeDescriptor.dstSet = m_descriptorSets[set][i];

					m_writeDescriptorsMapping[set][binding].emplace_back() = static_cast<uint32_t>(m_writeDescriptors.at(i).size() - 1);
				}
			}
		}
	}

	void VulkanDescriptorTable::InitializeInfoStructs()
	{
		VT_PROFILE_FUNCTION();

		const auto& resources = m_shader->GetResources();

		for (const auto& [set, bindings] : resources.uniformBuffers)
		{
			for (const auto& [binding, info] : bindings)
			{
				if (!m_bufferInfos[set].contains(binding))
				{
					m_bufferInfos[set][binding][0].resize(m_descriptorPoolCount);
				}

				for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
				{
					m_bufferInfos[set][binding][0][i].offset = 0;
					m_bufferInfos[set][binding][0][i].range = info.size;
				}
			}
		}

		for (const auto& [set, bindings] : resources.storageBuffers)
		{
			for (const auto& [binding, info] : bindings)
			{
				if (!m_bufferInfos[set].contains(binding))
				{
					m_bufferInfos[set][binding][0].resize(m_descriptorPoolCount);
				}

				for (uint32_t i = 0; i < m_descriptorPoolCount; i++)
				{
					m_bufferInfos[set][binding][0][i].offset = 0;
					m_bufferInfos[set][binding][0][i].range = info.size;
				}
			}
		}
	}
}
