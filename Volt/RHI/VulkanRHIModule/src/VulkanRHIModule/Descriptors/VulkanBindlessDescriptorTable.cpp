#include "vkpch.h"
#include "VulkanBindlessDescriptorTable.h"

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
		: m_image2DRegistry(1, framesInFlight), m_image2DArrayRegistry(1, framesInFlight), m_imageCubeRegistry(1, framesInFlight),
		m_bufferRegistry(1, framesInFlight), m_samplerRegistry(1, framesInFlight), m_image3DRegistry(1, framesInFlight)
	{
		m_activeDescriptorWrites.reserve(100);
		Invalidate();
	}

	VulkanBindlessDescriptorTable::~VulkanBindlessDescriptorTable()
	{
		Release();
	}

	ResourceHandle VulkanBindlessDescriptorTable::RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer)
	{
		VT_PROFILE_FUNCTION();
		return m_bufferRegistry.RegisterResource(storageBuffer);
	}

	ResourceHandle VulkanBindlessDescriptorTable::RegisterImageView(WeakPtr<ImageView> imageView)
	{
		VT_PROFILE_FUNCTION();
		const auto viewType = imageView->GetViewType();

		if (viewType == RHI::ImageViewType::View2D)
		{
			return m_image2DRegistry.RegisterResource(imageView, imageView->GetImageUsage());
		}
		else if (viewType == RHI::ImageViewType::View2DArray)
		{
			return m_image2DArrayRegistry.RegisterResource(imageView, imageView->GetImageUsage());
		}
		else if (viewType == RHI::ImageViewType::ViewCube)
		{
			return m_imageCubeRegistry.RegisterResource(imageView, imageView->GetImageUsage());
		}
		else if (viewType == ImageViewType::View3D)
		{
			return m_image3DRegistry.RegisterResource(imageView, imageView->GetImageUsage());
		}

		VT_ENSURE(false);
		return Resource::Invalid;
	}

	ResourceHandle VulkanBindlessDescriptorTable::RegisterSamplerState(WeakPtr<SamplerState> samplerState)
	{
		VT_PROFILE_FUNCTION();
		return m_samplerRegistry.RegisterResource(samplerState);
	}

	void VulkanBindlessDescriptorTable::UnregisterBuffer(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_bufferRegistry.UnregisterResource(handle);
	}

	void VulkanBindlessDescriptorTable::UnregisterImageView(ResourceHandle handle, ImageViewType viewType)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(viewType == RHI::ImageViewType::View2D || viewType == RHI::ImageViewType::View2DArray || viewType == RHI::ImageViewType::ViewCube || viewType == ImageViewType::View3D);

		if (viewType == RHI::ImageViewType::View2D)
		{
			m_image2DRegistry.UnregisterResource(handle);
		}
		else if (viewType == RHI::ImageViewType::View2DArray)
		{
			m_image2DArrayRegistry.UnregisterResource(handle);
		}
		else if (viewType == RHI::ImageViewType::ViewCube)
		{
			m_imageCubeRegistry.UnregisterResource(handle);
		}
		else if (viewType == ImageViewType::View3D)
		{
			m_image3DRegistry.UnregisterResource(handle);
		}
	}

	void VulkanBindlessDescriptorTable::UnregisterSamplerState(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_samplerRegistry.UnregisterResource(handle);
	}

	void VulkanBindlessDescriptorTable::MarkBufferAsDirty(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_bufferRegistry.MarkAsDirty(handle);
	}

	void VulkanBindlessDescriptorTable::MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType)
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE(viewType == RHI::ImageViewType::View2D || viewType == RHI::ImageViewType::View2DArray || viewType == RHI::ImageViewType::ViewCube);

		if (viewType == RHI::ImageViewType::View2D)
		{
			m_image2DRegistry.MarkAsDirty(handle);
		}
		else if (viewType == RHI::ImageViewType::View2DArray)
		{
			m_image2DArrayRegistry.MarkAsDirty(handle);
		}
		else if (viewType == RHI::ImageViewType::ViewCube)
		{
			m_imageCubeRegistry.MarkAsDirty(handle);
		}
		else if (viewType == RHI::ImageViewType::View3D)
		{
			m_image3DRegistry.MarkAsDirty(handle);
		}
	}

	void VulkanBindlessDescriptorTable::MarkSamplerStateAsDirty(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_samplerRegistry.MarkAsDirty(handle);
	}

	ResourceHandle VulkanBindlessDescriptorTable::GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer)
	{
		return m_bufferRegistry.GetResourceHandle(storageBuffer);
	}

	void VulkanBindlessDescriptorTable::Update()
	{
		VT_PROFILE_FUNCTION();

		m_image2DRegistry.Update();
		m_image2DArrayRegistry.Update();
		m_imageCubeRegistry.Update();
		m_image3DRegistry.Update();
		m_samplerRegistry.Update();
		m_bufferRegistry.Update();
	}

	void VulkanBindlessDescriptorTable::PrepareForRender()
	{
		VT_PROFILE_FUNCTION();

		// Buffers
		{
			std::scoped_lock lock{ m_bufferRegistry.GetMutex() };
			for (const auto& resourceHandle : m_bufferRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_bufferRegistry.GetResource(resourceHandle);
				auto storageBuffer = resourceData.resource.As<RHI::StorageBuffer>();

				DescriptorBufferInfo& bufferInfo = m_activeDescriptorBufferInfos.emplace_back();
				bufferInfo.range = storageBuffer->GetByteSize();
				bufferInfo.offset = 0;
				bufferInfo.buffer = storageBuffer->GetHandle<VkBuffer>();
			
				// Read Only
				{
					auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::BYTEADDRESSBUFFER_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&bufferInfo);
				}

				// Read-Write
				{
					auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::RWBYTEADDRESSBUFFER_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(&bufferInfo);
				}
			}
			m_bufferRegistry.ClearDirtyResources();
		}

		// Image2D
		{
			std::scoped_lock lock{ m_image2DRegistry.GetMutex() };
			for (const auto& resourceHandle : m_image2DRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_image2DRegistry.GetResource(resourceHandle);
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
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::TEXTURE2D_BINDING, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}

				// Read-Write
				if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
				{
					baseImageInfo.imageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_GENERAL);
					auto& imageInfo = m_activeDescriptorImageInfos.emplace_back(baseImageInfo);

					auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::RWTEXTURE2D_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}
			}
			m_image2DRegistry.ClearDirtyResources();
		}

		// Image2DArray
		{
			std::scoped_lock lock{ m_image2DArrayRegistry.GetMutex() };
			for (const auto& resourceHandle : m_image2DArrayRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_image2DArrayRegistry.GetResource(resourceHandle);
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
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::TEXTURE2DARRAY_BINDING, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}

				// Read-Write
				if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
				{
					baseImageInfo.imageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_GENERAL);
					auto& imageInfo = m_activeDescriptorImageInfos.emplace_back(baseImageInfo);

					auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::RWTEXTURE2DARRAY_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}
			}
			m_image2DArrayRegistry.ClearDirtyResources();
		}

		// ImageCube
		{
			std::scoped_lock lock{ m_imageCubeRegistry.GetMutex() };
			for (const auto& resourceHandle : m_imageCubeRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_imageCubeRegistry.GetResource(resourceHandle);
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
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::TEXTURECUBE_BINDING, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}
			}
			m_imageCubeRegistry.ClearDirtyResources();
		}

		// Image3D
		{
			std::scoped_lock lock{ m_image3DRegistry.GetMutex() };
			for (const auto& resourceHandle : m_image3DRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_image3DRegistry.GetResource(resourceHandle);
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
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::TEXTURE3D_BINDING, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}

				// Read-Write
				if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
				{
					baseImageInfo.imageLayout = static_cast<uint32_t>(VK_IMAGE_LAYOUT_GENERAL);
					auto& imageInfo = m_activeDescriptorImageInfos.emplace_back(baseImageInfo);

					auto& descriptorWrite = m_activeDescriptorWrites.emplace_back();
					Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::RWTEXTURE3D_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_mainDescriptorSet, resourceHandle.Get());
					descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
				}
			}
			m_image3DRegistry.ClearDirtyResources();
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
				Utility::InitializeDescriptorWrite(descriptorWrite, VulkanBindlessDescriptorLayoutManager::SAMPLERSTATE_BINDING, VK_DESCRIPTOR_TYPE_SAMPLER, m_mainDescriptorSet, resourceHandle.Get());
				descriptorWrite.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(&imageInfo);
			}
			m_samplerRegistry.ClearDirtyResources();
		}

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
		return m_mainDescriptorSet;
	}

	void VulkanBindlessDescriptorTable::SetOffsetIndexAndStride(const uint32_t offsetIndex, const uint32_t stride)
	{
		m_offsetIndex = offsetIndex;
		m_offsetStride = stride;
	}

	void VulkanBindlessDescriptorTable::SetConstantsBuffer(WeakPtr<UniformBuffer> constantsBuffer)
	{
		m_constantsBuffer = constantsBuffer;
	}

	void VulkanBindlessDescriptorTable::Bind(CommandBuffer& commandBuffer)
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
		const uint32_t alignedStride = Utility::Align(m_offsetStride, deviceProperties.limits.minUniformBufferOffsetAlignment);

		const bool hasConstantsSet = descriptorSetCount == 2;
		const uint32_t offset = alignedStride * m_offsetIndex;

		std::array<VkDescriptorSet, 2> descriptorSets = { m_mainDescriptorSet, hasConstantsSet ? GetOrAllocateConstantsSet() : nullptr };
		if (hasConstantsSet)
		{
			WriteConstantsSet(descriptorSets[1]);
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

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayouts.at(0);

		VT_VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, &m_mainDescriptorSet));
	}

	VkDescriptorSet_T* VulkanBindlessDescriptorTable::GetOrAllocateConstantsSet()
	{
		VT_PROFILE_FUNCTION();

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
		VT_VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, &resultSet));

		return resultSet;
	}

	void VulkanBindlessDescriptorTable::WriteConstantsSet(VkDescriptorSet_T* dstSet)
	{
		VT_PROFILE_FUNCTION();

		if (!m_constantsBuffer)
		{
			return;
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_constantsBuffer->GetHandle<VkBuffer>();
		bufferInfo.range = m_constantsBuffer->GetSize();
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
}
