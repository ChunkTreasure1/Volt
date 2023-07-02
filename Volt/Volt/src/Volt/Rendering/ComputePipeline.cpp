#include "vtpch.h"
#include "ComputePipeline.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Core/Graphics/GraphicsDevice.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Image3D.h"
#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Rendering/Buffer/UniformBuffer.h"
#include "Volt/Rendering/Buffer/UniformBufferSet.h"

#include "Volt/Rendering/Buffer/ShaderStorageBuffer.h"
#include "Volt/Rendering/Buffer/ShaderStorageBufferSet.h"

#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/GlobalDescriptorSetManager.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	ComputePipeline::ComputePipeline(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors)
		: myShader(computeShader), myCount(count), myUseGlobalDescriptors(useGlobalDescriptors)
	{
		Invalidate();

		Renderer::AddShaderDependency(computeShader, this);
	}

	ComputePipeline::ComputePipeline(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors, const ShaderDataBuffer& specializationConstants)
		: myShader(computeShader), myCount(count), myUseGlobalDescriptors(useGlobalDescriptors), myIsPermutation(true), mySpecializationConstantsBuffer(specializationConstants)
	{
		Invalidate();
		Renderer::AddShaderDependency(computeShader, this);
	}

	ComputePipeline::~ComputePipeline()
	{
		Renderer::RemoveShaderDependency(myShader, this);
		Release();
	}

	void ComputePipeline::Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, myPipeline);
	}

	void ComputePipeline::Dispatch(VkCommandBuffer commandBuffer, uint32_t groupX, uint32_t groupY, uint32_t groupZ, uint32_t index)
	{
		if (!myUseGlobalDescriptors)
		{
			AllocateDescriptorSets(index);
		}
		else
		{
			AllocateNonGlobalDescriptorSets(index);
		}

		UpdateDescriptorSets(commandBuffer, index);
		vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
	}

	void ComputePipeline::Clear(uint32_t index)
	{
		auto device = GraphicsContext::GetDevice();
		vkResetDescriptorPool(device->GetHandle(), myDescriptorPools.at(index), 0);
	}

	void ComputePipeline::Execute(uint32_t groupX, uint32_t groupY, uint32_t groupZ, const void* pushConstantData, const size_t pushConstantSize, uint32_t index)
	{
		auto device = GraphicsContext::GetDevice();
		Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(myCount, QueueType::Compute);
		vkResetDescriptorPool(device->GetHandle(), myDescriptorPools.at(index), 0);

		AllocateDescriptorSets(index);

		commandBuffer->Begin();

		UpdateDescriptorSets(commandBuffer->GetCurrentCommandBuffer(), index);

		if (pushConstantData)
		{
			PushConstants(commandBuffer->GetCurrentCommandBuffer(), pushConstantData, pushConstantSize);
		}

		vkCmdBindPipeline(commandBuffer->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, myPipeline);
		vkCmdDispatch(commandBuffer->GetCurrentCommandBuffer(), groupX, groupY, groupZ);
		InsertBarriers(commandBuffer->GetCurrentCommandBuffer(), index);
		commandBuffer->End();
		commandBuffer->Submit();
	}

	void ComputePipeline::InsertBarriers(VkCommandBuffer commandBuffer, uint32_t index)
	{
		if (myImageBarriers.at(index).empty() && myBufferBarriers.at(index).empty() && myExecutionBarriers.at(index).empty())
		{
			return;
		}

		const auto& bufferBarriers = myBufferBarriers.at(index);
		const auto& imageBarriers = myImageBarriers.at(index);
		const auto& executionBarriers = myExecutionBarriers.at(index);

		VkDependencyInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		info.pNext = nullptr;
		info.dependencyFlags = 0;
		info.memoryBarrierCount = (uint32_t)executionBarriers.size();
		info.pMemoryBarriers = executionBarriers.data();
		info.bufferMemoryBarrierCount = (uint32_t)bufferBarriers.size();
		info.pBufferMemoryBarriers = bufferBarriers.data();
		info.imageMemoryBarrierCount = (uint32_t)imageBarriers.size();
		info.pImageMemoryBarriers = imageBarriers.data();

		vkCmdPipelineBarrier2(commandBuffer, &info);
	}

	void ComputePipeline::PushConstants(VkCommandBuffer commandBuffer, const void* data, const size_t size)
	{
		vkCmdPushConstants(commandBuffer, myPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, (uint32_t)size, data);
	}

	void ComputePipeline::Invalidate()
	{
		Release();

		// Transfer pipeline generation data
		{
			auto oldGenerationData = mySpecializationConstantsBuffer;
			mySpecializationConstantsBuffer = {};
			if (myShader->GetResources().specializationConstantBuffers.contains(VK_SHADER_STAGE_COMPUTE_BIT))
			{
				auto newBuffer = myShader->CreateSpecialiazationConstantsBuffer(ShaderStage::Compute);

					const auto& oldMembers = oldGenerationData.GetMembers();

					for (const auto& [name, uniform] : newBuffer.GetMembers())
					{
						if (oldMembers.contains(name) && oldMembers.at(name).size == uniform.size)
						{
							newBuffer.SetValue(name, oldGenerationData.GetValueRaw(name, oldMembers.at(name).size), oldMembers.at(name).size);
						}
					}

					oldGenerationData.Release();

				mySpecializationConstantsBuffer = newBuffer;
			}
		}

		auto device = GraphicsContext::GetDevice();
		auto resources = myShader->GetResources();

		// Find global descriptor sets
		if (myUseGlobalDescriptors)
		{
			for (uint32_t set = 0; set < (uint32_t)resources.nullPaddedDescriptorSetLayouts.size(); set++)
			{
				if (GlobalDescriptorSetManager::HasDescriptorSet(set))
				{
					resources.nullPaddedDescriptorSetLayouts[set] = GlobalDescriptorSetManager::GetDescriptorSet(set)->GetDescriptorSetLayout();
				}
				else if (resources.descriptorSetBindings.contains(set))
				{
					myNonGlobalDescriptorSets.emplace_back(set);
				}
			}
		}

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = (uint32_t)resources.nullPaddedDescriptorSetLayouts.size();
		layoutInfo.pSetLayouts = resources.nullPaddedDescriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = resources.pushConstantRange.size > 0 ? 1 : 0;
		layoutInfo.pPushConstantRanges = &resources.pushConstantRange;

		VT_VK_CHECK(vkCreatePipelineLayout(device->GetHandle(), &layoutInfo, nullptr, &myPipelineLayout));

		auto computeStage = myShader->GetStageInfos().at(0);

		std::vector<VkSpecializationInfo> specConstInfos{};
		std::vector<std::vector<VkSpecializationMapEntry>> specConstEntries{};

		const auto specializationConstants = myShader->GetResources().specializationConstants;

		if (myIsPermutation)
		{
			specConstInfos.reserve(1);
			if (specializationConstants.contains(computeStage.stage))
			{
				const auto& specConsts = specializationConstants.at(computeStage.stage);
				auto& buffer = mySpecializationConstantsBuffer;

				VkSpecializationInfo& specConstInfo = specConstInfos.emplace_back();
				specConstInfo.mapEntryCount = buffer.GetMemberCount();
				specConstInfo.dataSize = buffer.GetSize();
				specConstInfo.pData = buffer.GetData();

				auto& entries = specConstEntries.emplace_back();

				entries.reserve(specConsts.size());
				for (const auto& [constantId, constantData] : specConsts)
				{
					entries.emplace_back(constantData.constantInfo);
				}

				specConstInfo.pMapEntries = entries.data();
				computeStage.pSpecializationInfo = &specConstInfo;
			}
		}

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = myPipelineLayout;
		pipelineInfo.flags = 0;
		pipelineInfo.stage = computeStage;

		VkPipelineCacheCreateInfo pipelineCacheInfo{};
		pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		VT_VK_CHECK(vkCreatePipelineCache(device->GetHandle(), &pipelineCacheInfo, nullptr, &myPipelineCache));
		VT_VK_CHECK(vkCreateComputePipelines(device->GetHandle(), myPipelineCache, 1, &pipelineInfo, nullptr, &myPipeline));

		CreateDescriptorPools();
		SetupFromShader();

		ResetAfterInvalidate();
	}

	void ComputePipeline::ClearAllResources()
	{
		myImage2DResources.clear();
		myImage3DResources.clear();
		myUniformBufferResources.clear();
		myStorageBufferResources.clear();

		myBufferBarriers.clear();
		myImageBarriers.clear();
		myExecutionBarriers.clear();

		myBufferBarriers.resize(myCount);
		myImageBarriers.resize(myCount);
		myExecutionBarriers.resize(myCount);

		myImageBarrierIndexMap.clear();
		myBufferBarrierIndexMap.clear();
	}

	void ComputePipeline::SetUniformBuffer(Ref<UniformBufferSet> uniformBufferSet, uint32_t set, uint32_t binding)
	{
		if (!mySetBindingWriteDescriptorMap.contains(set) || !mySetBindingWriteDescriptorMap.at(set).contains(binding))
		{
			return;
		}

		const uint32_t writeIndex = mySetBindingWriteDescriptorMap.at(set).at(binding);

		if (myWriteDescriptors.at(0).at(writeIndex).descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
			myWriteDescriptors.at(0).at(writeIndex).descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			VT_CORE_ERROR("[ComputePipeline] Trying to set uniform buffer to descriptor set of non uniform buffer type!");
			return;
		}

		for (uint32_t index = 0; auto & resources : myShaderResources)
		{
			auto& ubo = resources.GetUniformBufferAt(set, binding);
			ubo.info.buffer = uniformBufferSet->Get(set, binding, index)->GetHandle();
			index++;
		}

		myUniformBufferResources[set][binding] = uniformBufferSet;
	}

	void ComputePipeline::SetStorageBuffer(Ref<ShaderStorageBufferSet> shaderStorageBufferSet, uint32_t set, uint32_t binding)
	{
		if (!mySetBindingWriteDescriptorMap.contains(set) || !mySetBindingWriteDescriptorMap.at(set).contains(binding))
		{
			return;
		}

		const uint32_t writeIndex = mySetBindingWriteDescriptorMap.at(set).at(binding);

		if (myWriteDescriptors.at(0).at(writeIndex).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
			myWriteDescriptors.at(0).at(writeIndex).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
		{
			VT_CORE_ERROR("[ComputePipeline] Trying to set storage buffer to descriptor set of non storage buffer type!");
			return;
		}

		for (uint32_t index = 0; auto & resources : myShaderResources)
		{
			auto& ssbo = resources.GetStorageBufferAt(set, binding);
			ssbo.info.buffer = shaderStorageBufferSet->Get(set, binding, index)->GetHandle();
			ssbo.info.offset = 0;
			ssbo.info.range = shaderStorageBufferSet->Get(set, binding, index)->GetSize();
			index++;
		}

		myStorageBufferResources[set][binding] = shaderStorageBufferSet;
	}

	void ComputePipeline::SetImage(Ref<Image2D> image, uint32_t set, uint32_t binding, ImageAccess imageAccess)
	{
		SetImage(image, set, binding, 0, imageAccess);
	}

	void ComputePipeline::SetImage(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t mip, ImageAccess imageAccess)
	{
		if (!mySetBindingWriteDescriptorMap.contains(set) || !mySetBindingWriteDescriptorMap.at(set).contains(binding))
		{
			return;
		}

		const uint32_t index = mySetBindingWriteDescriptorMap.at(set).at(binding);

		if (myWriteDescriptors.at(0).at(index).descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
			myWriteDescriptors.at(0).at(index).descriptorType != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE &&
			myWriteDescriptors.at(0).at(index).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		{
			VT_CORE_ERROR("[ComputePipeline] Trying to set image to descriptor set of non image type!");
			return;
		}

		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (imageAccess == ImageAccess::Write)
		{
			imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else
		{
			if (Utility::IsDepthFormat(image->GetSpecification().format))
			{
				imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			}
			else
			{
				imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
		}

		auto setInfoFunc = [=](VkDescriptorImageInfo& info, Ref<Image2D> image, uint32_t, uint32_t)
		{
			info.imageLayout = imageLayout;

			// Special case for RWTexture2DArray images that actually are cube maps
			if (image->GetSpecification().isCubeMap && myWriteDescriptors.at(0).at(index).descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			{
				info.imageView = image->GetArrayView(mip);
			}
			else
			{
				info.imageView = image->GetView(mip);
			}

			info.sampler = image->GetSampler();
		};

		for (auto& resources : myShaderResources)
		{
			if (resources.HasSampledImageAt(set, binding))
			{
				setInfoFunc(resources.GetSampledImageAt(set, binding).info, image, set, binding);
			}
			else if (resources.HasStorageImageAt(set, binding))
			{
				setInfoFunc(resources.GetStorageImageAt(set, binding).info, image, set, binding);
			}
			else if (resources.HasSeparateImageAt(set, binding))
			{
				setInfoFunc(resources.GetSeparateImageAt(set, binding).info, image, set, binding);
			}
		}

		myImage2DResources[set][binding] = { image, imageLayout, imageAccess };
	}

	void ComputePipeline::SetImage(Ref<Image2D>, uint32_t, uint32_t, uint32_t, uint32_t, ImageAccess)
	{

	}

	void ComputePipeline::SetImage(Ref<Image3D> image, uint32_t set, uint32_t binding, ImageAccess imageAccess)
	{
		SetImage(image, set, binding, 0, imageAccess);
	}

	void ComputePipeline::SetImage(Ref<Image3D> image, uint32_t set, uint32_t binding, uint32_t mip, ImageAccess imageAccess)
	{
		if (!mySetBindingWriteDescriptorMap.contains(set) || !mySetBindingWriteDescriptorMap.at(set).contains(binding))
		{
			return;
		}

		const uint32_t index = mySetBindingWriteDescriptorMap.at(set).at(binding);

		if (myWriteDescriptors.at(0).at(index).descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
			myWriteDescriptors.at(0).at(index).descriptorType != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE &&
			myWriteDescriptors.at(0).at(index).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		{
			VT_CORE_ERROR("[ComputePipeline] Trying to set image to descriptor set of non image type!");
			return;
		}

		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (imageAccess == ImageAccess::Write)
		{
			imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else
		{
			if (Utility::IsDepthFormat(image->GetSpecification().format))
			{
				imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			}
			else
			{
				imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
		}

		auto setInfoFunc = [&](VkDescriptorImageInfo& info, Ref<Image3D> image, uint32_t set, uint32_t binding)
		{
			info.imageLayout = imageLayout;
			info.imageView = image->GetView(mip);
			info.sampler = image->GetSampler();
		};

		for (auto& resources : myShaderResources)
		{
			if (resources.HasSampledImageAt(set, binding))
			{
				setInfoFunc(resources.GetSampledImageAt(set, binding).info, image, set, binding);
			}
			else if (resources.HasStorageImageAt(set, binding))
			{
				setInfoFunc(resources.GetStorageImageAt(set, binding).info, image, set, binding);
			}
			else if (resources.HasSeparateImageAt(set, binding))
			{
				setInfoFunc(resources.GetSeparateImageAt(set, binding).info, image, set, binding);
			}
		}

		myImage3DResources[set][binding] = { image, imageLayout, imageAccess };
	}

	void ComputePipeline::InsertImageBarrier(uint32_t set, uint32_t binding, const ImageBarrierInfo& barrierInfo)
	{
		const bool is2DImage = (myImage2DResources.contains(set) && myImage2DResources.at(set).contains(binding));
		const bool is3DImage = (myImage3DResources.contains(set) && myImage3DResources.at(set).contains(binding));

		if (!is2DImage && !is3DImage)
		{
			VT_CORE_WARN("[ComputePipeline] Unable to set image barrier at set {0} binding {1} because it does not exist!", set, binding);
			return;
		}

		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		barrier.srcStageMask = barrierInfo.srcStageMask;
		barrier.srcAccessMask = barrierInfo.srcAccessMask;
		barrier.dstStageMask = barrierInfo.dstStageMask;
		barrier.dstAccessMask = barrierInfo.dstAccessMask;
		barrier.newLayout = barrierInfo.newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange = barrierInfo.subresourceRange;

		if (is2DImage)
		{
			const auto& image = myImage2DResources.at(set).at(binding);
			barrier.oldLayout = image.imageLayout;
			barrier.image = image.image->GetHandle();
		}
		else
		{
			const auto& image = myImage3DResources.at(set).at(binding);
			barrier.oldLayout = image.imageLayout;
			barrier.image = image.image->GetHandle();
		}


		if (myImageBarrierIndexMap.contains(set) && myImageBarrierIndexMap.at(set).contains(binding))
		{
			const uint32_t barrierIndex = myImageBarrierIndexMap.at(set).at(binding);
			for (uint32_t i = 0; i < myCount; i++)
			{
				myImageBarriers.at(i)[barrierIndex] = barrier;
			}
		}
		else
		{
			const uint32_t barrierIndex = (uint32_t)myImageBarriers.at(0).size();
			for (uint32_t i = 0; i < myCount; i++)
			{
				myImageBarriers.at(i).emplace_back(barrier);
			}

			myImageBarrierIndexMap[set][binding] = barrierIndex;
		}
	}

	void ComputePipeline::InsertUniformBufferBarrier(uint32_t set, uint32_t binding, const BufferBarrierInfo& barrierInfo)
	{
		if (!myUniformBufferResources.contains(set) || !myUniformBufferResources.at(set).contains(binding))
		{
			VT_CORE_WARN("[ComputePipeline] Unable to set buffer barrier at set {0} binding {1} because it does not exist!", set, binding);
			return;
		}

		VkBufferMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		barrier.srcStageMask = barrierInfo.srcStageMask;
		barrier.srcAccessMask = barrierInfo.srcAccessMask;
		barrier.dstStageMask = barrierInfo.dstStageMask;
		barrier.dstAccessMask = barrierInfo.dstAccessMask;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.offset = barrierInfo.offset;
		barrier.size = barrierInfo.size;

		if (myBufferBarrierIndexMap.contains(set) && myBufferBarrierIndexMap.at(set).contains(binding))
		{
			const uint32_t barrierIndex = myBufferBarrierIndexMap.at(set).at(binding);
			for (uint32_t i = 0; i < myCount; i++)
			{
				const auto ubo = myUniformBufferResources.at(set).at(binding)->Get(set, binding, i);

				barrier.buffer = ubo->GetHandle();
				myBufferBarriers.at(i)[barrierIndex] = barrier;
			}
		}
		else
		{
			const uint32_t barrierIndex = (uint32_t)myBufferBarriers.at(0).size();
			for (uint32_t i = 0; i < myCount; i++)
			{
				const auto ubo = myUniformBufferResources.at(set).at(binding)->Get(set, binding, i);

				barrier.buffer = ubo->GetHandle();
				myBufferBarriers.at(i)[barrierIndex] = barrier;
			}

			myBufferBarrierIndexMap[set][binding] = barrierIndex;
		}
	}

	void ComputePipeline::InsertStorageBufferBarrier(uint32_t set, uint32_t binding, const BufferBarrierInfo& barrierInfo)
	{
		if (!myStorageBufferResources.contains(set) || !myStorageBufferResources.at(set).contains(binding))
		{
			VT_CORE_WARN("[ComputePipeline] Unable to set buffer barrier at set {0} binding {1} because it does not exist!", set, binding);
			return;
		}

		VkBufferMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		barrier.srcStageMask = barrierInfo.srcStageMask;
		barrier.srcAccessMask = barrierInfo.srcAccessMask;
		barrier.dstStageMask = barrierInfo.dstStageMask;
		barrier.dstAccessMask = barrierInfo.dstAccessMask;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.offset = barrierInfo.offset;
		barrier.size = barrierInfo.size;

		if (myBufferBarrierIndexMap.contains(set) && myBufferBarrierIndexMap.at(set).contains(binding))
		{
			const uint32_t barrierIndex = myBufferBarrierIndexMap.at(set).at(binding);
			for (uint32_t i = 0; i < myCount; i++)
			{
				const auto ssbo = myStorageBufferResources.at(set).at(binding)->Get(set, binding, i);

				barrier.buffer = ssbo->GetHandle();
				myBufferBarriers.at(i)[barrierIndex] = barrier;
			}
		}
		else
		{
			const uint32_t barrierIndex = (uint32_t)myBufferBarriers.at(0).size();
			for (uint32_t i = 0; i < myCount; i++)
			{
				const auto ssbo = myStorageBufferResources.at(set).at(binding)->Get(set, binding, i);

				barrier.buffer = ssbo->GetHandle();
				myBufferBarriers.at(i).emplace_back(barrier);
			}

			myBufferBarrierIndexMap[set][binding] = barrierIndex;
		}
	}

	void ComputePipeline::InsertStorageBufferBarrier(Ref<ShaderStorageBufferSet> shaderStorageBufferSet, uint32_t set, uint32_t binding, const BufferBarrierInfo& barrierInfo)
	{
		VkBufferMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		barrier.srcStageMask = barrierInfo.srcStageMask;
		barrier.srcAccessMask = barrierInfo.srcAccessMask;
		barrier.dstStageMask = barrierInfo.dstStageMask;
		barrier.dstAccessMask = barrierInfo.dstAccessMask;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.offset = barrierInfo.offset;
		barrier.size = barrierInfo.size;

		if (myBufferBarrierIndexMap.contains(set) && myBufferBarrierIndexMap.at(set).contains(binding))
		{
			const uint32_t barrierIndex = myBufferBarrierIndexMap.at(set).at(binding);
			for (uint32_t i = 0; i < myCount; i++)
			{
				const auto ssbo = shaderStorageBufferSet->Get(set, binding, i);

				barrier.buffer = ssbo->GetHandle();
				myBufferBarriers.at(i)[barrierIndex] = barrier;
			}
		}
		else
		{
			const uint32_t barrierIndex = (uint32_t)myBufferBarriers.at(0).size();
			for (uint32_t i = 0; i < myCount; i++)
			{
				const auto ssbo = shaderStorageBufferSet->Get(set, binding, i);

				barrier.buffer = ssbo->GetHandle();
				myBufferBarriers.at(i).emplace_back(barrier);
			}

			myBufferBarrierIndexMap[set][binding] = barrierIndex;
		}
	}

	void ComputePipeline::InsertExecutionBarrier(const ExecutionBarrierInfo& barrierInfo)
	{
		for (uint32_t i = 0; i < myCount; i++)
		{
			auto& barrier = myExecutionBarriers.at(i).emplace_back();
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
			barrier.pNext = nullptr;

			barrier.srcStageMask = barrierInfo.srcStageMask;
			barrier.srcAccessMask = barrierInfo.srcAccessMask;

			barrier.dstStageMask = barrierInfo.dstStageMask;
			barrier.dstAccessMask = barrierInfo.dstAccessMask;
		}
	}

	void ComputePipeline::BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, myPipelineLayout, set, 1, &descriptorSet, 0, nullptr);
	}

	bool ComputePipeline::HasDescriptorSet(uint32_t set) const
	{
		const auto& resources = myShaderResources.at(0);
		return resources.descriptorSetBindings.contains(set); // Descriptor set bindings contains all the sets in the shader
	}

	Ref<ComputePipeline> ComputePipeline::Create(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors)
	{
		return CreateRef<ComputePipeline>(computeShader, count, useGlobalDescriptors);
	}

	Ref<ComputePipeline> ComputePipeline::Create(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors, const ShaderDataBuffer& specializationConstants)
	{
		return CreateRef<ComputePipeline>(computeShader, count, useGlobalDescriptors, specializationConstants);
	}

	void ComputePipeline::Release()
	{
		if (!myPipeline)
		{
			return;
		}

		Renderer::SubmitResourceChange([pipelineLayout = myPipelineLayout, pipelineCache = myPipelineCache, pipeline = myPipeline, descriptorPools = myDescriptorPools]()
		{
			auto device = GraphicsContext::GetDevice();
			for (const auto& descriptorPool : descriptorPools)
			{
				vkDestroyDescriptorPool(device->GetHandle(), descriptorPool, nullptr);
			}

			vkDestroyPipelineCache(device->GetHandle(), pipelineCache, nullptr);
			vkDestroyPipeline(device->GetHandle(), pipeline, nullptr);
			vkDestroyPipelineLayout(device->GetHandle(), pipelineLayout, nullptr);
		});

		myDescriptorPools.clear();
		myDescriptorSets.clear();
		myNonGlobalDescriptorSets.clear();
		myWriteDescriptors.clear();
		mySetBindingWriteDescriptorMap.clear();
		myWriteDescriptorSetMapping.clear();
	}

	void ComputePipeline::SetupFromShader()
	{
		auto device = GraphicsContext::GetDevice();
		const auto& shaderResources = myShader->GetResources();

		myShaderResources = { myCount, shaderResources };
		myDescriptorSets.resize(myCount);
		myWriteDescriptors.resize(myCount);
		myBufferBarriers.resize(myCount);
		myImageBarriers.resize(myCount);
		myExecutionBarriers.resize(myCount);

		// Copy and set write descriptor infos

		for (uint32_t index = 0; auto & resources : myShaderResources)
		{
			for (const auto& [set, bindings] : resources.writeDescriptors)
			{
				if (myUseGlobalDescriptors)
				{
					if (std::find(myNonGlobalDescriptorSets.begin(), myNonGlobalDescriptorSets.end(), set) == myNonGlobalDescriptorSets.end())
					{
						continue;
					}
				}

				for (const auto& [binding, writeDescriptor] : bindings)
				{
					const uint32_t lastIndex = (uint32_t)myWriteDescriptors.at(index).size();
					auto& newWriteDescriptor = myWriteDescriptors.at(index).emplace_back(writeDescriptor);

					if (index == 0)
					{
						myWriteDescriptorSetMapping.emplace_back(set);
					}
					mySetBindingWriteDescriptorMap[set][binding] = lastIndex;

					if (resources.HasUniformBufferAt(set, binding))
					{
						newWriteDescriptor.pBufferInfo = &resources.GetUniformBufferAt(set, binding).info;
					}
					else if (resources.HasStorageBufferAt(set, binding))
					{
						newWriteDescriptor.pBufferInfo = &resources.GetStorageBufferAt(set, binding).info;
					}
					else if (resources.HasSampledImageAt(set, binding))
					{
						newWriteDescriptor.pImageInfo = &resources.GetSampledImageAt(set, binding).info;
					}
					else if (resources.HasStorageImageAt(set, binding))
					{
						newWriteDescriptor.pImageInfo = &resources.GetStorageImageAt(set, binding).info;
					}
					else if (resources.HasSeparateImageAt(set, binding))
					{
						newWriteDescriptor.pImageInfo = &resources.GetSeparateImageAt(set, binding).info;
					}
				}
			}

			index++;
		}
	}

	void ComputePipeline::CreateDescriptorPools()
	{
		const auto& resources = myShader->GetResources();

		auto poolSizes = resources.descriptorPoolSizes;
		for (auto& poolSize : poolSizes)
		{
			poolSize.descriptorCount *= 10;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = 0;
		poolInfo.maxSets = 100;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();

		auto device = GraphicsContext::GetDevice();

		for (uint32_t i = 0; i < myCount; i++)
		{
			VT_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &myDescriptorPools.emplace_back()));
		}
	}

	void ComputePipeline::AllocateDescriptorSets(uint32_t index)
	{
		auto device = GraphicsContext::GetDevice();

		auto& resources = myShaderResources.at(index);
		auto& descriptorSets = myDescriptorSets.at(index);
		descriptorSets.resize(resources.descriptorSetAllocateInfo.descriptorSetCount);

		VkDescriptorSetAllocateInfo allocInfo = resources.descriptorSetAllocateInfo;
		allocInfo.descriptorPool = myDescriptorPools.at(index);
		vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, descriptorSets.data());
	}

	void ComputePipeline::UpdateDescriptorSets(VkCommandBuffer commandBuffer, uint32_t index)
	{
		auto device = GraphicsContext::GetDevice();

		if (myWriteDescriptors.at(index).empty())
		{
			return;
		}

		for (uint32_t currentSetIndex = 0, i = 0; i < (uint32_t)myWriteDescriptorSetMapping.size(); i++)
		{
			if (i > 0 && myWriteDescriptorSetMapping.at(i) != myWriteDescriptorSetMapping.at(i - 1))
			{
				currentSetIndex++;
			}

			auto set = myDescriptorSets.at(index).at(currentSetIndex);
			myWriteDescriptors.at(index).at(i).dstSet = set;
		}

		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)myWriteDescriptors.at(index).size(), myWriteDescriptors.at(index).data(), 0, nullptr);

		// We use myWriteDescriptorSetMapping.at(0) because this will indirectly be the first existing set
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, myPipelineLayout, myWriteDescriptorSetMapping.at(0), (uint32_t)myDescriptorSets.at(index).size(), myDescriptorSets.at(index).data(), 0, nullptr);
	}

	void ComputePipeline::AllocateNonGlobalDescriptorSets(uint32_t index)
	{
		if (myNonGlobalDescriptorSets.empty())
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();
		auto& descriptorSets = myDescriptorSets.at(index);
		descriptorSets.clear();

		for (const auto& set : myNonGlobalDescriptorSets)
		{
			VkDescriptorSet descriptorSet = myShader->AllocateDescriptorSet(set, myDescriptorPools.at(index));
			descriptorSets.emplace_back(descriptorSet);
		}
	}

	void ComputePipeline::ResetAfterInvalidate()
	{
		for (const auto& [set, bindings] : myStorageBufferResources)
		{
			for (const auto& [binding, storageBufferSet] : bindings)
			{
				SetStorageBuffer(storageBufferSet, set, binding);
			}
		}

		for (const auto& [set, bindings] : myUniformBufferResources)
		{
			for (const auto& [binding, uniformBufferSet] : bindings)
			{
				SetUniformBuffer(uniformBufferSet, set, binding);
			}
		}

		for (const auto& [set, bindings] : myImage2DResources)
		{
			for (const auto& [binding, image] : bindings)
			{
				SetImage(image.image, set, binding, image.access);
			}
		}

		for (const auto& [set, bindings] : myImage3DResources)
		{
			for (const auto& [binding, image] : bindings)
			{
				SetImage(image.image, set, binding, image.access);
			}
		}
	}
}
