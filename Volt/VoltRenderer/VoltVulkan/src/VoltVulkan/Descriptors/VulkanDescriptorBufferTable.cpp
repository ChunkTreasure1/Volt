#include "vkpch.h"
#include "VulkanDescriptorBufferTable.h"

#include "VoltVulkan/Common/VulkanFunctions.h"
#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

#include "VoltVulkan/Buffers/VulkanBufferView.h"
#include "VoltVulkan/Buffers/VulkanStorageBuffer.h"
#include "VoltVulkan/Buffers/VulkanUniformBuffer.h"
#include "VoltVulkan/Buffers/VulkanCommandBuffer.h"

#include <VoltRHI/Core/Profiling.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Images/SamplerState.h>

#include <VoltRHI/Memory/Allocation.h>
#include <VoltRHI/Memory/MemoryUtility.h>

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

	VulkanDescriptorBufferTable::VulkanDescriptorBufferTable(const DescriptorTableCreateInfo& createInfo)
	{
		m_shader = createInfo.shader;
		m_descriptorBufferCount = createInfo.count;

		// Find descriptor offsets
		{
			const auto& descriptorBufferProperties = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().GetProperties().descriptorBufferProperties;

			m_descriptorTypeOffsets.uboOffset = Utility::Align(descriptorBufferProperties.uniformBufferDescriptorSize, descriptorBufferProperties.descriptorBufferOffsetAlignment);
			m_descriptorTypeOffsets.ssboOffset = Utility::Align(descriptorBufferProperties.storageBufferDescriptorSize, descriptorBufferProperties.descriptorBufferOffsetAlignment);
			m_descriptorTypeOffsets.storageImageOffset = Utility::Align(descriptorBufferProperties.storageImageDescriptorSize, descriptorBufferProperties.descriptorBufferOffsetAlignment);
			m_descriptorTypeOffsets.imageOffset = Utility::Align(descriptorBufferProperties.sampledImageDescriptorSize, descriptorBufferProperties.descriptorBufferOffsetAlignment);
			m_descriptorTypeOffsets.samplerOffset = Utility::Align(descriptorBufferProperties.samplerDescriptorSize, descriptorBufferProperties.descriptorBufferOffsetAlignment);

			m_descriptorTypeOffsets.uboSize = descriptorBufferProperties.uniformBufferDescriptorSize;
			m_descriptorTypeOffsets.ssboSize = descriptorBufferProperties.storageBufferDescriptorSize;
			m_descriptorTypeOffsets.storageImageSize = descriptorBufferProperties.storageImageDescriptorSize;
			m_descriptorTypeOffsets.imageSize = descriptorBufferProperties.sampledImageDescriptorSize;
			m_descriptorTypeOffsets.samplerSize = descriptorBufferProperties.samplerDescriptorSize;
		}

		Invalidate();
	}

	VulkanDescriptorBufferTable::~VulkanDescriptorBufferTable()
	{
		Release();
	}

	void VulkanDescriptorBufferTable::SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_descriptorSetBindingOffsets.contains(set))
		{
			return;
		}

		if (!m_descriptorSetBindingOffsets.at(set).contains(binding))
		{
			return;
		}

		const VkDescriptorType descriptorType = static_cast<VkDescriptorType>(m_imageDescriptorTypes.at(set).at(binding));

		VkDescriptorImageInfo imageDescriptor;
		imageDescriptor.imageView = imageView->GetHandle<VkImageView>();
		imageDescriptor.sampler = nullptr;
		imageDescriptor.imageLayout = Utility::GetImageLayoutFromDescriptorType(descriptorType);

		VkDescriptorGetInfoEXT imageDescriptorInfo{};
		imageDescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
		imageDescriptorInfo.pNext = nullptr;
		imageDescriptorInfo.type = descriptorType;

		uint64_t descriptorTypeSize = 0;

		if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		{
			imageDescriptorInfo.data.pStorageImage = &imageDescriptor;

			descriptorTypeSize = m_descriptorTypeOffsets.storageImageSize;
		}
		else if (descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		{
			imageDescriptorInfo.data.pSampledImage = &imageDescriptor;

			descriptorTypeSize = m_descriptorTypeOffsets.imageSize;
		}

		const uint64_t bufferOffset = m_descriptorSetBindingOffsets.at(set).at(binding) + descriptorTypeSize * arrayIndex;
		void* descriptorPtr = m_hostDescriptorBuffer.As<void>(bufferOffset);

		VT_ENSURE(bufferOffset < m_accumulatedSize);
		vkGetDescriptorEXT(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &imageDescriptorInfo, descriptorTypeSize, descriptorPtr);
	}

	void VulkanDescriptorBufferTable::SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_descriptorSetBindingOffsets.contains(set))
		{
			return;
		}

		if (!m_descriptorSetBindingOffsets.at(set).contains(binding))
		{
			return;
		}

		VkDescriptorAddressInfoEXT addressInfo{};
		addressInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
		addressInfo.pNext = nullptr;
		addressInfo.format = VK_FORMAT_UNDEFINED;
		addressInfo.address = bufferView->GetDeviceAddress();

		Ref<RHIResource> rawResource = bufferView->AsRef<VulkanBufferView>().GetResource();

		uint64_t descriptorTypeSize = 0;
		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		if (rawResource->GetType() == ResourceType::StorageBuffer)
		{
			addressInfo.range = rawResource->AsRef<VulkanStorageBuffer>().GetByteSize();
			descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorTypeSize = m_descriptorTypeOffsets.ssboSize;
		}
		else if (rawResource->GetType() == ResourceType::UniformBuffer)
		{
			addressInfo.range = rawResource->AsRef<VulkanUniformBuffer>().GetSize();
			descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorTypeSize = m_descriptorTypeOffsets.uboSize;
		}

		VkDescriptorGetInfoEXT bufferDescriptorInfo{};
		bufferDescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
		bufferDescriptorInfo.type = descriptorType;

		if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		{
			bufferDescriptorInfo.data.pStorageBuffer = &addressInfo;
		}
		else if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			bufferDescriptorInfo.data.pUniformBuffer = &addressInfo;
		}

		const uint64_t bufferOffset = m_descriptorSetBindingOffsets.at(set).at(binding) + descriptorTypeSize * arrayIndex;
		void* descriptorPtr = m_hostDescriptorBuffer.As<void>(bufferOffset);

		VT_ENSURE(bufferOffset < m_accumulatedSize);
		vkGetDescriptorEXT(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &bufferDescriptorInfo, descriptorTypeSize, descriptorPtr);
	}

	void VulkanDescriptorBufferTable::SetBufferViewSet(Ref<BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
	}

	void VulkanDescriptorBufferTable::SetImageView(std::string_view name, Ref<ImageView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetImageView(view, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorBufferTable::SetBufferView(std::string_view name, Ref<BufferView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetBufferView(view, binding.set, binding.binding, arrayIndex);
	}

	void VulkanDescriptorBufferTable::SetBufferViews(const std::vector<Ref<BufferView>>& bufferViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset)
	{
		for (uint32_t index = arrayStartOffset; const auto & view : bufferViews)
		{
			SetBufferView(view, set, binding, index);
			index++;
		}
	}

	void VulkanDescriptorBufferTable::SetImageViews(const std::vector<Ref<ImageView>>& imageViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset)
	{
		for (uint32_t index = arrayStartOffset; const auto & view : imageViews)
		{
			SetImageView(view, set, binding, index);
			index++;
		}
	}

	void VulkanDescriptorBufferTable::SetSamplerState(Ref<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_descriptorSetBindingOffsets.contains(set))
		{
			return;
		}

		if (!m_descriptorSetBindingOffsets.at(set).contains(binding))
		{
			return;
		}

		VkSampler sampler = samplerState->GetHandle<VkSampler>();

		VkDescriptorGetInfoEXT imageDescriptorInfo{};
		imageDescriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
		imageDescriptorInfo.pNext = nullptr;
		imageDescriptorInfo.type = VK_DESCRIPTOR_TYPE_SAMPLER;
		imageDescriptorInfo.data.pSampler = &sampler;

		const uint64_t descriptorTypeSize = m_descriptorTypeOffsets.samplerSize;

		const uint64_t bufferOffset = m_descriptorSetBindingOffsets.at(set).at(binding) + descriptorTypeSize * arrayIndex;
		void* descriptorPtr = m_hostDescriptorBuffer.As<void>(bufferOffset);

		vkGetDescriptorEXT(GraphicsContext::GetDevice()->GetHandle<VkDevice>(), &imageDescriptorInfo, descriptorTypeSize, descriptorPtr);
	}

	void* VulkanDescriptorBufferTable::GetHandleImpl() const
	{
		return nullptr;
	}

	void VulkanDescriptorBufferTable::Bind(Ref<CommandBuffer> commandBuffer)
	{
		std::vector<VkDescriptorBufferBindingInfoEXT> descriptorBindingBufferInfos{};

		VulkanCommandBuffer& vulkanCommandBuffer = commandBuffer->AsRef<VulkanCommandBuffer>();
		const VkPipelineBindPoint bindPoint = vulkanCommandBuffer.m_currentRenderPipeline ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

		{
			void* buff = m_descriptorBuffer->Map<void*>();

			memcpy(buff, m_hostDescriptorBuffer.GetData(), m_accumulatedSize);

			m_descriptorBuffer->Unmap();
		}

		for (const auto& set : m_shader->GetResources().usedSets)
		{
			set;

			auto& bindingInfo = descriptorBindingBufferInfos.emplace_back();
			bindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
			bindingInfo.address = m_descriptorBuffer->GetDeviceAddress();
			bindingInfo.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		}

		vkCmdBindDescriptorBuffersEXT(commandBuffer->GetHandle<VkCommandBuffer>(), static_cast<uint32_t>(descriptorBindingBufferInfos.size()), descriptorBindingBufferInfos.data());

		constexpr uint32_t bufferIndex = 0;
		for (const auto& set : m_usedDescriptorSets)
		{
			const uint64_t offset = m_descriptorSetOffsets.at(set);
			vkCmdSetDescriptorBufferOffsetsEXT(commandBuffer->GetHandle<VkCommandBuffer>(), bindPoint, vulkanCommandBuffer.GetCurrentPipelineLayout(), set, 1, &bufferIndex, &offset);
		}
	}

	void VulkanDescriptorBufferTable::Invalidate()
	{
		Release();

		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		Ref<VulkanShader> vulkanShader = m_shader->As<VulkanShader>();

		for (const auto& descriptorSetLayout : vulkanShader->GetPaddedDescriptorSetLayouts())
		{
			vkGetDescriptorSetLayoutSizeEXT(device->GetHandle<VkDevice>(), descriptorSetLayout, &m_descriptorSetLayoutSizes.emplace_back());
		}

		uint64_t currentSize = 0;
		for (uint32_t set = 0; const auto & size : m_descriptorSetLayoutSizes)
		{
			m_descriptorSetOffsets[set++] = currentSize;
			currentSize += size;
		}

		for (const auto& set : m_shader->GetResources().usedSets)
		{
			m_usedDescriptorSets.emplace_back(set);
		}

		const uint64_t accumulatedSize = std::accumulate(m_descriptorSetLayoutSizes.begin(), m_descriptorSetLayoutSizes.end(), uint64_t(0));
		m_descriptorBuffer = GraphicsContext::GetDefaultAllocator().CreateBuffer(accumulatedSize, BufferUsage::DescriptorBuffer, MemoryUsage::CPUToGPU);
		m_hostDescriptorBuffer.Resize(accumulatedSize);

		m_accumulatedSize = accumulatedSize;

		CalculateDescriptorOffsets();
	}

	void VulkanDescriptorBufferTable::Release()
	{
		GraphicsContext::DestroyResource([descriptorBuffer = m_descriptorBuffer]()
		{
			if (descriptorBuffer)
			{
				GraphicsContext::GetDefaultAllocator().DestroyBuffer(descriptorBuffer);
			}
		});

		m_descriptorBuffer = nullptr;
		m_descriptorSetLayoutSizes.clear();
		m_usedDescriptorSets.clear();
		m_descriptorSetOffsets.clear();
		m_descriptorSetBindingOffsets.clear();
		m_imageDescriptorTypes.clear();
		m_hostDescriptorBuffer.Release();
	}

	void VulkanDescriptorBufferTable::CalculateDescriptorOffsets()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();

		const auto& resources = m_shader->GetResources();
		Ref<VulkanShader> vulkanShader = m_shader->As<VulkanShader>();
		const auto& descriptorSetLayouts = vulkanShader->GetPaddedDescriptorSetLayouts();

		for (const auto& [set, bindings] : resources.uniformBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.uboSize;
			}
		}

		for (const auto& [set, bindings] : resources.storageBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				if (data.arraySize == -1)
				{
					m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.ssboSize * VulkanDefaults::STORAGE_BUFFER_BINDLESS_TABLE_SIZE;
				}
				else
				{
					m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.ssboSize * data.arraySize;
				}
			}
		}

		for (const auto& [set, bindings] : resources.storageImages)
		{
			for (const auto& [binding, data] : bindings)
			{
				if (data.arraySize == -1)
				{
					m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.storageImageSize * VulkanDefaults::STORAGE_IMAGE_BINDLESS_TABLE_SIZE;
				}
				else
				{
					m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.storageImageSize * data.arraySize;
				}

				m_imageDescriptorTypes[set][binding] = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			}
		}

		for (const auto& [set, bindings] : resources.images)
		{
			for (const auto& [binding, data] : bindings)
			{
				if (data.arraySize == -1)
				{
					m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.imageSize * VulkanDefaults::IMAGE_BINDLESS_TABLE_SIZE;
				}
				else
				{
					m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.imageSize * data.arraySize;
				}

				m_imageDescriptorTypes[set][binding] = static_cast<uint32_t>(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			}
		}

		for (const auto& [set, bindings] : resources.samplers)
		{
			for (const auto& [binding, data] : bindings)
			{
				m_descriptorSetBindingOffsets[set][binding] = m_descriptorTypeOffsets.samplerSize;
			}
		}

		for (auto& [set, bindings] : m_descriptorSetBindingOffsets)
		{
			const uint64_t setBaseOffset = m_descriptorSetOffsets.at(set);
			//uint64_t currentOffset = 0;

			for (auto& [binding, offset] : bindings)
			{
				//const uint64_t descriptorSize = offset;

				vkGetDescriptorSetLayoutBindingOffsetEXT(device->GetHandle<VkDevice>(), descriptorSetLayouts.at(set), binding, &offset);

				offset += setBaseOffset;
				//currentOffset += descriptorSize;
			}
		}
	}
}
