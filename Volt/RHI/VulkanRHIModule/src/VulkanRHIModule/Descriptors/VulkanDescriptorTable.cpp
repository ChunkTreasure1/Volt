#include "vkpch.h"
#include "VulkanDescriptorTable.h"

#include "VulkanRHIModule/Shader/VulkanShader.h"
#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorLayoutManager.h"
#include "VulkanRHIModule/Buffers/VulkanBufferView.h"
#include "VulkanRHIModule/Buffers/VulkanStorageBuffer.h"
#include "VulkanRHIModule/Buffers/VulkanCommandBuffer.h"

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Images/ImageView.h>
#include <RHIModule/Images/SamplerState.h>

#include <RHIModule/RHIProxy.h>

#include <CoreUtilities/Profiling/Profiling.h>

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

	VulkanDescriptorTable::VulkanDescriptorTable(const DescriptorTableCreateInfo& createInfo)
	{
		m_shader = createInfo.shader;
		Invalidate();
	}

	VulkanDescriptorTable::~VulkanDescriptorTable()
	{
		Release();
	}

	void VulkanDescriptorTable::SetImageView(WeakPtr<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			VT_LOGC(Warning, LogVulkanRHI, "Trying to assign image view at set {0} and binding {1}. But that is not a valid binding!", set, binding);
			return;
		}

		m_isDirty = true;

		auto& description = m_imageDescriptorInfos[set][binding][arrayIndex];
		description.imageView = imageView->GetHandle<VkImageView>();
		description.sampler = nullptr;

		VT_ENSURE(description.imageView);

		uint32_t writeDescriptorIndex = 0;

		if (m_activeDescriptorWritesMapping[set][binding][arrayIndex].value == DefaultInvalid::INVALID_VALUE)
		{
			writeDescriptorIndex = m_writeDescriptorsMapping[set][binding];

			DescriptorWrite writeDescriptorCopy = m_descriptorWrites.at(writeDescriptorIndex);
			writeDescriptorCopy.dstArrayElement = arrayIndex;
			writeDescriptorCopy.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);
			m_activeDescriptorWrites.emplace_back(writeDescriptorCopy);

			writeDescriptorIndex = static_cast<uint32_t>(m_activeDescriptorWrites.size() - 1);
			m_activeDescriptorWritesMapping[set][binding][arrayIndex].value = writeDescriptorIndex;
		}
		else
		{
			writeDescriptorIndex = m_activeDescriptorWritesMapping[set][binding][arrayIndex].value;
			m_activeDescriptorWrites.at(writeDescriptorIndex).pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);
		}

		description.imageLayout = Utility::GetImageLayoutFromDescriptorType(static_cast<VkDescriptorType>(m_activeDescriptorWrites.at(writeDescriptorIndex).descriptorType));
	}

	void VulkanDescriptorTable::SetBufferView(WeakPtr<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			VT_LOGC(Warning, LogVulkanRHI, "Trying to assign buffer view at set {0} and binding {1}. But that is not a valid binding!", set, binding);
			return;
		}

		m_isDirty = true;

		VulkanBufferView& vkBufferView = bufferView->AsRef<VulkanBufferView>();

		auto& description = m_bufferDescriptorInfos[set][binding][arrayIndex];
		description.buffer = vkBufferView.GetHandle<VkBuffer>();

		VT_ENSURE(description.buffer);

		auto bufferResource = vkBufferView.GetResource();
		if (bufferResource->GetType() == ResourceType::StorageBuffer)
		{
			description.range = bufferResource->GetByteSize();
		}

		if (m_activeDescriptorWritesMapping[set][binding][arrayIndex].value == DefaultInvalid::INVALID_VALUE)
		{
			const uint32_t writeDescriptorIndex = m_writeDescriptorsMapping[set][binding];

			DescriptorWrite writeDescriptorCopy = m_descriptorWrites.at(writeDescriptorIndex);
			writeDescriptorCopy.dstArrayElement = arrayIndex;
			writeDescriptorCopy.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&description);
			m_activeDescriptorWrites.emplace_back(writeDescriptorCopy);

			const uint32_t activeWriteDescriptorIndex = static_cast<uint32_t>(m_activeDescriptorWrites.size() - 1);
			m_activeDescriptorWritesMapping[set][binding][arrayIndex].value = activeWriteDescriptorIndex;
		}
		else
		{
			const uint32_t writeDescriptorIndex = m_activeDescriptorWritesMapping[set][binding][arrayIndex].value;
			m_activeDescriptorWrites.at(writeDescriptorIndex).pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&description);
		}
	}

	void VulkanDescriptorTable::SetSamplerState(WeakPtr<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_writeDescriptorsMapping[set].contains(binding))
		{
			VT_LOGC(Warning, LogVulkanRHI, "Trying to assign sampler state at set {0} and binding {1}. But that is not a valid binding!", set, binding);
			return;
		}

		m_isDirty = true;

		auto& description = m_imageDescriptorInfos[set][binding][arrayIndex];
		description.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		description.imageView = nullptr;
		description.sampler = samplerState->GetHandle<VkSampler>();

		VT_ENSURE(description.sampler);


		if (m_activeDescriptorWritesMapping[set][binding][arrayIndex].value == DefaultInvalid::INVALID_VALUE)
		{
			const uint32_t writeDescriptorIndex = m_writeDescriptorsMapping[set][binding];

			DescriptorWrite writeDescriptorCopy = m_descriptorWrites.at(writeDescriptorIndex);
			writeDescriptorCopy.dstArrayElement = arrayIndex;
			writeDescriptorCopy.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);
			m_activeDescriptorWrites.emplace_back(writeDescriptorCopy);

			const uint32_t activeWriteDescriptorIndex = static_cast<uint32_t>(m_activeDescriptorWrites.size() - 1);
			m_activeDescriptorWritesMapping[set][binding][arrayIndex].value = activeWriteDescriptorIndex;
		}
		else
		{
			const uint32_t writeDescriptorIndex = m_activeDescriptorWritesMapping[set][binding][arrayIndex].value;
			m_activeDescriptorWrites.at(writeDescriptorIndex).pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&description);
		}
	}

	void VulkanDescriptorTable::SetImageView(std::string_view name, WeakPtr<ImageView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetImageView(view, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorTable::SetBufferView(std::string_view name, WeakPtr<BufferView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetBufferView(view, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorTable::SetSamplerState(std::string_view name, WeakPtr<SamplerState> samplerState, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetSamplerState(samplerState, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorTable::PrepareForRender()
	{
		if (!m_isDirty)
		{
			return;
		}

		if (m_activeDescriptorWrites.empty())
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();
		const VkWriteDescriptorSet* writeDescriptorsPtr = reinterpret_cast<const VkWriteDescriptorSet*>(m_activeDescriptorWrites.data());

		vkUpdateDescriptorSets(device->GetHandle<VkDevice>(), static_cast<uint32_t>(m_activeDescriptorWrites.size()), writeDescriptorsPtr, 0, nullptr);

		m_activeDescriptorWrites.clear();
		m_activeDescriptorWritesMapping.clear();

		m_isDirty = false;
	}

	void VulkanDescriptorTable::Bind(CommandBuffer& commandBuffer)
	{
		VulkanCommandBuffer& vulkanCommandBuffer = commandBuffer.AsRef<VulkanCommandBuffer>();
		const VkPipelineBindPoint bindPoint = vulkanCommandBuffer.m_currentRenderPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

		PrepareForRender();

		// #TODO_Ivar: move to an implementation that binds all descriptor sets in one call
		for (const auto& [setIndex, set] : m_descriptorSets)
		{
			vkCmdBindDescriptorSets(vulkanCommandBuffer.GetHandle<VkCommandBuffer>(), bindPoint, vulkanCommandBuffer.GetCurrentPipelineLayout(), setIndex, 1, &set, 0, nullptr);
		}
	}

	void* VulkanDescriptorTable::GetHandleImpl() const
	{
		return nullptr;
	}

	void VulkanDescriptorTable::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		Release();

		m_imageDescriptorInfos.clear();
		m_bufferDescriptorInfos.clear();

		VulkanShader& vulkanShader = m_shader->AsRef<VulkanShader>();
		const auto& shaderPoolSizes = vulkanShader.GetDescriptorPoolSizes();

		uint32_t maxSets = 0;

		Vector<VkDescriptorPoolSize> poolSizes{};
		for (const auto& [type, count] : shaderPoolSizes)
		{
			poolSizes.emplace_back(static_cast<VkDescriptorType>(type), count);
			maxSets += count;
		}

		[[maybe_unused]] auto device = GraphicsContext::GetDevice();

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.maxSets = maxSets;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		VT_VK_CHECK(vkCreateDescriptorPool(device->GetHandle<VkDevice>(), &poolInfo, nullptr, &m_descriptorPool));

		const auto& usedSets = vulkanShader.GetResources().usedSets;

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorSetCount = 1;

		for (const auto& set : usedSets)
		{
			allocInfo.descriptorPool = m_descriptorPool;
			allocInfo.pSetLayouts = &vulkanShader.GetPaddedDescriptorSetLayouts().at(set);

			VT_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle<VkDevice>(), &allocInfo, &m_descriptorSets[set]));
		}

		BuildWriteDescriptors();
		InitializeInfoStructs();
	}

	void VulkanDescriptorTable::Release()
	{
		if (!m_descriptorPool)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([descriptorPool = m_descriptorPool]()
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroyDescriptorPool(device->GetHandle<VkDevice>(), descriptorPool, nullptr);
		});

		m_descriptorPool = nullptr;
	}

	void VulkanDescriptorTable::BuildWriteDescriptors()
	{
		const auto& resources = m_shader->GetResources();

		m_descriptorWrites.clear();
		m_activeDescriptorWrites.clear();

		for (const auto& [set, bindings] : resources.uniformBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& writeDescriptor = m_descriptorWrites.emplace_back();
				InitilizeWriteDescriptor(writeDescriptor, binding, static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER), m_descriptorSets[set]);
				m_writeDescriptorsMapping[set][binding] = static_cast<uint32_t>(m_descriptorWrites.size() - 1);
			}
		}

		for (const auto& [set, bindings] : resources.storageBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& writeDescriptor = m_descriptorWrites.emplace_back();
				InitilizeWriteDescriptor(writeDescriptor, binding, static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER), m_descriptorSets[set]);
				m_writeDescriptorsMapping[set][binding] = static_cast<uint32_t>(m_descriptorWrites.size() - 1);
			}
		}

		for (const auto& [set, bindings] : resources.storageImages)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& writeDescriptor = m_descriptorWrites.emplace_back();
				InitilizeWriteDescriptor(writeDescriptor, binding, static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE), m_descriptorSets[set]);
				m_writeDescriptorsMapping[set][binding] = static_cast<uint32_t>(m_descriptorWrites.size() - 1);
			}
		}

		for (const auto& [set, bindings] : resources.images)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& writeDescriptor = m_descriptorWrites.emplace_back();
				InitilizeWriteDescriptor(writeDescriptor, binding, static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE), m_descriptorSets[set]);
				m_writeDescriptorsMapping[set][binding] = static_cast<uint32_t>(m_descriptorWrites.size() - 1);
			}
		}

		for (const auto& [set, bindings] : resources.samplers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& writeDescriptor = m_descriptorWrites.emplace_back();
				InitilizeWriteDescriptor(writeDescriptor, binding, static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLER), m_descriptorSets[set]);
				m_writeDescriptorsMapping[set][binding] = static_cast<uint32_t>(m_descriptorWrites.size() - 1);
			}
		}
	}

	void VulkanDescriptorTable::InitializeInfoStructs()
	{
		const auto& resources = m_shader->GetResources();

		for (const auto& [set, bindings] : resources.uniformBuffers)
		{
			for (const auto& [binding, info] : bindings)
			{
				m_bufferDescriptorInfos[set][binding][0].offset = 0;
				m_bufferDescriptorInfos[set][binding][0].range = info.size;
			}
		}

		for (const auto& [set, bindings] : resources.storageBuffers)
		{
			for (const auto& [binding, info] : bindings)
			{
				m_bufferDescriptorInfos[set][binding][0].offset = 0;
				m_bufferDescriptorInfos[set][binding][0].range = info.size;
			}
		}
	}

	void VulkanDescriptorTable::InitilizeWriteDescriptor(DescriptorWrite& writeDescriptor, const uint32_t binding, const uint32_t descriptorType, VkDescriptorSet_T* dstDescriptorSet)
	{
		writeDescriptor.sType = static_cast<uint32_t>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptor.pNext = nullptr;
		writeDescriptor.descriptorCount = 1;
		writeDescriptor.dstArrayElement = 0;
		writeDescriptor.dstBinding = binding;
		writeDescriptor.descriptorType = descriptorType;
		writeDescriptor.dstSet = dstDescriptorSet;
	}
}
