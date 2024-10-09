#include "vkpch.h"
#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorTable.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorLayoutManager.h"
#include "VulkanRHIModule/Buffers/VulkanCommandBuffer.h"
#include "VulkanRHIModule/Graphics/VulkanPhysicalGraphicsDevice.h"

#include <RHIModule/RHIProxy.h>
#include <RHIModule/Graphics/GraphicsContext.h>

#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Images/ImageView.h>
#include <RHIModule/Images/SamplerState.h>
#include <RHIModule/Pipelines/RenderPipeline.h>
#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Shader/Shader.h>
#include <RHIModule/Memory/MemoryUtility.h>

#include <CoreUtilities/ComparisonHelpers.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline void InitializeDescriptorWrite(DescriptorWrite& descriptorWrite, const uint32_t binding, VkDescriptorType descriptorType, VkDescriptorSet_T* dstDescriptorSet, const uint32_t arrayIndex)
		{
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.pNext = nullptr;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.dstArrayElement = arrayIndex;
			descriptorWrite.dstBinding = binding;
			descriptorWrite.descriptorType = descriptorType;
			descriptorWrite.dstSet = dstDescriptorSet;
		}

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

	VulkanBindlessDescriptorTable::VulkanBindlessDescriptorTable(uint64_t framesInFlight)
		: m_mainRegistry(2, framesInFlight), m_samplerRegistry(1, framesInFlight), m_framesInFlight(framesInFlight)
	{
		m_activeDescriptorWrites.reserve(100);
		m_mainDescriptorSets.resize(framesInFlight, nullptr);

		Invalidate();
	}

	VulkanBindlessDescriptorTable::~VulkanBindlessDescriptorTable()
	{
		Release();
	}

	ResourceHandle VulkanBindlessDescriptorTable::RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer)
	{
		VT_PROFILE_FUNCTION();
		return m_mainRegistry.RegisterResource(storageBuffer, ImageUsage::None, static_cast<uint32_t>(ResourceType::StorageBuffer));
	}

	ResourceHandle VulkanBindlessDescriptorTable::RegisterImageView(WeakPtr<ImageView> imageView)
	{
		VT_PROFILE_FUNCTION();

		return m_mainRegistry.RegisterResource(imageView, imageView->GetImageUsage(), static_cast<uint32_t>(ResourceType::Image1D));

		VT_ENSURE(false);
		return Resource::Invalid;
	}

	ResourceHandle VulkanBindlessDescriptorTable::RegisterSamplerState(WeakPtr<SamplerState> samplerState)
	{
		VT_PROFILE_FUNCTION();
		return m_samplerRegistry.RegisterResource(samplerState);
	}

	void VulkanBindlessDescriptorTable::UnregisterResource(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.UnregisterResource(handle);
	}

	void VulkanBindlessDescriptorTable::MarkResourceAsDirty(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.MarkAsDirty(handle);
	}

	void VulkanBindlessDescriptorTable::UnregisterSamplerState(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_samplerRegistry.UnregisterResource(handle);
	}

	void VulkanBindlessDescriptorTable::MarkSamplerStateAsDirty(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_samplerRegistry.MarkAsDirty(handle);
	}

	void VulkanBindlessDescriptorTable::Update()
	{
		VT_PROFILE_FUNCTION();

		m_mainRegistry.Update();
		m_samplerRegistry.Update();

		m_frameIndex = (m_frameIndex + 1) % m_framesInFlight;
	}

	void VulkanBindlessDescriptorTable::PrepareForRender()
	{
		PrepareHeapForRender();

		if (m_activeDescriptorWrites.empty())
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();
		const VkWriteDescriptorSet* writeDescriptorsPtr = reinterpret_cast<const VkWriteDescriptorSet*>(m_activeDescriptorWrites.data());

		vkUpdateDescriptorSets(device->GetHandle<VkDevice>(), static_cast<uint32_t>(m_activeDescriptorWrites.size()), writeDescriptorsPtr, 0, nullptr);

		m_activeDescriptorWrites.clear();
		m_activeDescriptorImageInfos.clear();
		m_activeDescriptorBufferInfos.clear();
	}

	void* VulkanBindlessDescriptorTable::GetHandleImpl() const
	{
		return GetCurrentMainDescriptorSet();
	}

	void VulkanBindlessDescriptorTable::Bind(CommandBuffer& commandBuffer, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();
		VulkanCommandBuffer& vulkanCommandBuffer = commandBuffer.AsRef<VulkanCommandBuffer>();

		VkPipelineBindPoint bindPoint;
		uint32_t descriptorSetCount = 0;

		if (vulkanCommandBuffer.m_currentRenderPipeline)
		{
			descriptorSetCount = vulkanCommandBuffer.m_currentRenderPipeline->GetShader()->GetResources().renderGraphConstantsData.IsValid() ? 2 : 1;
			bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		}
		else
		{
			descriptorSetCount = vulkanCommandBuffer.m_currentComputePipeline->GetShader()->GetResources().renderGraphConstantsData.IsValid() ? 2 : 1;
			bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		}

		const auto& deviceProperties = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>()->GetProperties();
		const uint32_t alignedStride = Utility::Align(stride, deviceProperties.limits.minUniformBufferOffsetAlignment);

		const bool hasConstantsSet = descriptorSetCount == 2;
		const uint32_t offset = alignedStride * offsetIndex;

		std::array<VkDescriptorSet, 2> descriptorSets = { GetCurrentMainDescriptorSet(), hasConstantsSet ? GetOrAllocateConstantsSet() : nullptr };
		if (hasConstantsSet)
		{
			WriteConstantsSet(descriptorSets[1], constantsBuffer);
		}

		vkCmdBindDescriptorSets(vulkanCommandBuffer.GetHandle<VkCommandBuffer>(), bindPoint, vulkanCommandBuffer.GetCurrentPipelineLayout(), 0, descriptorSetCount, descriptorSets.data(), hasConstantsSet ? 1 : 0, &offset);

		if (hasConstantsSet)
		{
			WeakPtr<VulkanBindlessDescriptorTable> tablePtr = this;

			RHIProxy::GetInstance().DestroyResource([tablePtr, descriptor = descriptorSets[1]]()
			{
				if (tablePtr)
				{
					tablePtr->m_availiableConstantsSet.emplace_back(descriptor);
				}
			});
		}
	}

	void VulkanBindlessDescriptorTable::Release()
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
	}

	void VulkanBindlessDescriptorTable::Invalidate()
	{
		Release();

		constexpr VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10000 },
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.maxSets = 100'000;
		poolInfo.poolSizeCount = 6;
		poolInfo.pPoolSizes = poolSizes;

		auto vkDevice = GraphicsContext::GetDevice()->GetHandle<VkDevice>();

		VT_VK_CHECK(vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &m_descriptorPool));

		const auto descriptorSetLayouts = VulkanBindlessDescriptorLayoutManager::GetGlobalDescriptorSetLayouts();

		Vector<VkDescriptorSetLayout> setLayouts(m_framesInFlight, descriptorSetLayouts.at(0));

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(m_framesInFlight);
		allocInfo.pSetLayouts = setLayouts.data();

		VT_VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, m_mainDescriptorSets.data()));
	}

	void VulkanBindlessDescriptorTable::PrepareHeapForRender()
	{
		VT_PROFILE_FUNCTION();

		// Main heap
		{
			std::scoped_lock lock{ m_mainRegistry.GetMutex() };
			for (const auto& resourceHandle : m_mainRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_mainRegistry.GetResource(resourceHandle);
				const ResourceType resourceType = static_cast<ResourceType>(resourceData.userData);

				if (resourceType == ResourceType::StorageBuffer)
				{
					auto storageBuffer = resourceData.resource.As<RHI::StorageBuffer>();

					DescriptorBufferInfo& bufferInfo = m_activeDescriptorBufferInfos.emplace_back();
					bufferInfo.range = storageBuffer->GetByteSize();
					bufferInfo.offset = 0;
					bufferInfo.buffer = storageBuffer->GetHandle<VkBuffer>();

					// Read Only
					{
						auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
						Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::CBV_SRV_UAV_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, GetCurrentMainDescriptorSet(), resourceHandle.Get());
						descriptorWrite.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&bufferInfo);
					}

					// Read-Write
					{
						ResourceHandle rwHandle = resourceHandle + ResourceHandle(1u);

						auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
						Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::CBV_SRV_UAV_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, GetCurrentMainDescriptorSet(), rwHandle);
						descriptorWrite.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&bufferInfo);
					}
				}
				// Image1D in this case means all image types.
				else if (resourceType == ResourceType::Image1D)
				{
					VT_ENSURE(resourceData.imageUsage != RHI::ImageUsage::None);

					const auto imageView = resourceData.resource.As<RHI::ImageView>();

					DescriptorImageInfo baseImageInfo{};
					baseImageInfo.sampler = nullptr;
					baseImageInfo.imageView = imageView->GetHandle<VkImageView>();

					// Read Only
					{
						baseImageInfo.imageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						auto& imageInfo = m_activeDescriptorImageInfos.emplace_back(baseImageInfo);

						auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
						Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::CBV_SRV_UAV_BINDING, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, GetCurrentMainDescriptorSet(), resourceHandle.Get());
						descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
					}

					// Read-Write
					if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
					{
						baseImageInfo.imageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_GENERAL);
						auto& imageInfo = m_activeDescriptorImageInfos.emplace_back(baseImageInfo);

						ResourceHandle rwHandle = resourceHandle + ResourceHandle(1u);

						auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
						Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::CBV_SRV_UAV_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, GetCurrentMainDescriptorSet(), rwHandle);
						descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
					}
				}
			}

			m_mainRegistry.ClearDirtyResources();
		}

		// Samplers
		{
			std::scoped_lock lock{ m_samplerRegistry.GetMutex() };
			for (const auto& resourceHandle : m_samplerRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_samplerRegistry.GetResource(resourceHandle);
				const auto samplerState = resourceData.resource.As<RHI::SamplerState>();

				DescriptorImageInfo& imageInfo = m_activeDescriptorImageInfos.emplace_back();
				imageInfo.sampler = samplerState->GetHandle<VkSampler>();

				auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
				Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::SAMPLERS_BINDING, VK_DESCRIPTOR_TYPE_SAMPLER, GetCurrentMainDescriptorSet(), resourceHandle.Get());
				descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
			}
			m_samplerRegistry.ClearDirtyResources();
		}
	}

	VkDescriptorSet_T* VulkanBindlessDescriptorTable::GetOrAllocateConstantsSet()
	{
		VT_PROFILE_FUNCTION();
		std::scoped_lock lock{ m_descriptorAllocationMutex };

		if (!m_availiableConstantsSet.empty())
		{
			auto set = m_availiableConstantsSet.back();
			m_availiableConstantsSet.pop_back();
			return set;
		}

		const auto descriptorSetLayouts = VulkanBindlessDescriptorLayoutManager::GetGlobalDescriptorSetLayouts();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayouts.at(1);

		VkDescriptorSet resultSet;

		auto vkDevice = GraphicsContext::GetDevice()->GetHandle<VkDevice>();

		{
			VT_VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, &resultSet));
		}

		return resultSet;
	}

	void VulkanBindlessDescriptorTable::WriteConstantsSet(VkDescriptorSet_T* dstSet, WeakPtr<UniformBuffer> constantsBuffer)
	{
		VT_PROFILE_FUNCTION();

		if (!constantsBuffer)
		{
			return;
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = constantsBuffer->GetHandle<VkBuffer>();
		bufferInfo.range = constantsBuffer->GetSize();
		bufferInfo.offset = 0;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.pNext = nullptr;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.dstBinding = 998;
		descriptorWrite.dstSet = dstSet;
		descriptorWrite.pBufferInfo = &bufferInfo;

		auto vkDevice = GraphicsContext::GetDevice()->GetHandle<VkDevice>();
		vkUpdateDescriptorSets(vkDevice, 1, &descriptorWrite, 0, nullptr);
	}

	VkDescriptorSet_T* VulkanBindlessDescriptorTable::GetCurrentMainDescriptorSet() const
	{
		return m_mainDescriptorSets.at(m_frameIndex);
	}
}
