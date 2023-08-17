#include "vkpch.h"
#include "VulkanComputePipeline.h"

#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanComputePipeline::VulkanComputePipeline(Ref<Shader> shader)
		: m_shader(shader)
	{
		Invalidate();
	}
	
	VulkanComputePipeline::~VulkanComputePipeline()
	{
		Release();
	}
	
	void VulkanComputePipeline::Invalidate()
	{
		Release();

		auto device = GraphicsContext::GetDevice();
		const auto& shaderResources = m_shader->GetResources();
		Ref<VulkanShader> vulkanShader = m_shader->As<VulkanShader>();

		// Create Pipeline Layout
		{
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.size = shaderResources.constants.size;
			pushConstantRange.offset = shaderResources.constants.offset;
			pushConstantRange.stageFlags = static_cast<VkShaderStageFlags>(shaderResources.constants.stageFlags);

			assert(pushConstantRange.size <= 128 && "Push constant range must be less or equal to 128 bytes to support all platforms!");

			const auto& setLayouts = vulkanShader->GetPaddedDescriptorSetLayouts();

			VkPipelineLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
			info.pSetLayouts = setLayouts.data();
			info.pushConstantRangeCount = shaderResources.constants.size > 0 ? 1 : 0;
			info.pPushConstantRanges = &pushConstantRange;

			VT_VK_CHECK(vkCreatePipelineLayout(device->GetHandle<VkDevice>(), &info, nullptr, &m_pipelineLayout));
		}

		// Create Pipeline
		{
			const auto& firstStage = vulkanShader->GetPipelineStageInfos().at(ShaderStage::Compute);

			VkPipelineShaderStageCreateInfo computeStageInfo{};
			computeStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			computeStageInfo.pNext = nullptr;
			computeStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			computeStageInfo.module = firstStage.shaderModule;
			computeStageInfo.pName = "main"; // #TODO_Ivar: Set correct entry point

			VkComputePipelineCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			info.pNext = nullptr;
			info.layout = m_pipelineLayout;
			info.flags = 0;
			info.stage = computeStageInfo;
			info.basePipelineHandle = nullptr;
			info.basePipelineIndex = 0;

			VT_VK_CHECK(vkCreateComputePipelines(device->GetHandle<VkDevice>(), nullptr, 1, &info, nullptr, &m_pipeline));
		}
	}

	void* VulkanComputePipeline::GetHandleImpl()
	{
		return m_pipeline;
	}
	
	void VulkanComputePipeline::Release()
	{
		if (!m_pipeline)
		{
			return;
		}

		GraphicsContext::DestroyResource([pipeline = m_pipeline, pipelineLayout = m_pipelineLayout]() 
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroyPipeline(device->GetHandle<VkDevice>(), pipeline, nullptr);
			vkDestroyPipelineLayout(device->GetHandle<VkDevice>(), pipelineLayout, nullptr);
		});

		m_pipeline = nullptr;
		m_pipelineLayout = nullptr;
	}
}
