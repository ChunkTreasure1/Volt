#include "vkpch.h"
#include "VulkanRenderPipeline.h"

#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Common/VulkanHelpers.h"
#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	struct VertexAttributeData
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	namespace Utility
	{
		static VertexAttributeData CreateVertexLayout(const BufferLayout& bufferLayout)
		{
			assert(!bufferLayout.GetElements().empty());

			VertexAttributeData result{};

			VkVertexInputBindingDescription& bindingDesc = result.bindingDescriptions.emplace_back();
			bindingDesc.binding = 0;
			bindingDesc.stride = bufferLayout.GetStride();
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			uint32_t attributeIndex = 0;
			for (const auto& element : bufferLayout.GetElements())
			{
				VkVertexInputAttributeDescription& desc = result.attributeDescriptions.emplace_back();
				desc.binding = 0;
				desc.location = attributeIndex;
				desc.format = VoltToVulkanElementFormat(element.type);
				desc.offset = static_cast<uint32_t>(element.offset);

				attributeIndex++;
			}

			// #TODO: instance layout

			return result;
		}
	}

	VulkanRenderPipeline::VulkanRenderPipeline(const RenderPipelineCreateInfo& createInfo)
		: m_createInfo(createInfo)
	{
		Invalidate();
	}
	
	VulkanRenderPipeline::~VulkanRenderPipeline()
	{
		Release();
	}
	
	void VulkanRenderPipeline::Invalidate()
	{
		Release();

		auto device = GraphicsContext::GetDevice();
		const auto& shaderResources = m_createInfo.shader->GetResources();
		Ref<VulkanShader> vulkanShader = m_createInfo.shader->As<VulkanShader>();
		
		VertexAttributeData vertexAttrData{};
		
		if (shaderResources.vertexLayout.IsValid())
		{
			vertexAttrData = Utility::CreateVertexLayout(shaderResources.vertexLayout);
		}

		// Create pipeline layout
		{
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.size = shaderResources.constants.size;
			pushConstantRange.offset = shaderResources.constants.offset;
			pushConstantRange.stageFlags = static_cast<VkShaderStageFlags>(shaderResources.constants.stageFlags);

			assert(pushConstantRange.size <= 128 && "Push constant range must be less or equal to 128 bytes to support all platforms!");

			const auto& setLayouts = vulkanShader->GetPaddedDescriptorSetLayouts();

			VkPipelineLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
			info.pSetLayouts = setLayouts.data();
			info.pushConstantRangeCount = shaderResources.constants.size > 0 ? 1 : 0;
			info.pPushConstantRanges = &pushConstantRange;
			
			VT_VK_CHECK(vkCreatePipelineLayout(device->GetHandle<VkDevice>(), &info, nullptr, &m_pipelineLayout));
		}

		// Create Pipeline
		{
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexAttrData.bindingDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = vertexAttrData.bindingDescriptions.data();

			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrData.attributeDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttrData.attributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.topology = Utility::VoltToVulkanTopology(m_createInfo.topology);
			inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo viewportInfo{};
			viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportInfo.viewportCount = 1;
			viewportInfo.pViewports = nullptr;
			viewportInfo.scissorCount = 1;
			viewportInfo.pScissors = nullptr;

			VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
			rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizerInfo.depthBiasClamp = VK_FALSE;
			rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizerInfo.polygonMode = Utility::VoltToVulkanFill(m_createInfo.fillMode);
			rasterizerInfo.cullMode = Utility::VoltToVulkanCull(m_createInfo.cullMode);
			rasterizerInfo.lineWidth = 1.f;
			rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizerInfo.depthBiasEnable = VK_FALSE;

			// #TODO_Ivar: Add tessellation support

			VkPipelineMultisampleStateCreateInfo multiSampleInfo{};
			multiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multiSampleInfo.sampleShadingEnable = VK_FALSE;
			multiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineColorBlendStateCreateInfo blendInfo{};
			blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blendInfo.logicOpEnable = VK_FALSE;
			blendInfo.logicOp = VK_LOGIC_OP_COPY;
			blendInfo.blendConstants[0] = 0.f;
			blendInfo.blendConstants[1] = 0.f;
			blendInfo.blendConstants[2] = 0.f;
			blendInfo.blendConstants[3] = 0.f;

			std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{};
			for (const auto& outputFormat : shaderResources.outputFormats)
			{
				outputFormat;

				VkPipelineColorBlendAttachmentState& blendAttachment = blendAttachments.emplace_back();
				blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				blendAttachment.blendEnable = VK_FALSE;
			}
			
			// #TODO_Ivar: Add blend attachments
			blendInfo.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
			blendInfo.pAttachments = blendAttachments.data();

			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
			depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilInfo.depthCompareOp = Utility::VoltToVulkanCompareOp(m_createInfo.depthCompareOperator);
			depthStencilInfo.stencilTestEnable = VK_FALSE;
			depthStencilInfo.depthBoundsTestEnable = VK_FALSE;

			switch (m_createInfo.depthMode)
			{
				case DepthMode::None:
				{
					depthStencilInfo.depthTestEnable = VK_FALSE;
					depthStencilInfo.depthWriteEnable = VK_FALSE;
					break;
				}

				case DepthMode::Read:
				{
					depthStencilInfo.depthTestEnable = VK_TRUE;
					depthStencilInfo.depthWriteEnable = VK_FALSE;
					break;
				}

				case DepthMode::Write:
				{
					depthStencilInfo.depthTestEnable = VK_FALSE;
					depthStencilInfo.depthWriteEnable = VK_TRUE;
					break;
				}

				case DepthMode::ReadWrite:
				{
					depthStencilInfo.depthTestEnable = VK_TRUE;
					depthStencilInfo.depthWriteEnable = VK_TRUE;
					break;
				}
			}

			VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
			dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

			std::vector<VkDynamicState> dynamicStates =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
			};

			dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicStateInfo.pDynamicStates = dynamicStates.data();

			VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
			pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
			pipelineRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(shaderResources.outputFormats.size());
			pipelineRenderingInfo.pColorAttachmentFormats = reinterpret_cast<const VkFormat*>(shaderResources.outputFormats.data());

			// #TODO_Ivar: How are we supposed to find these?
			//pipelineRenderingInfo.depthAttachmentFormat;
			//pipelineRenderingInfo.stencilAttachmentFormat;
		
			std::vector<VkPipelineShaderStageCreateInfo> pipelineStageInfos{};

			for (const auto& [stage, stageInfo] : vulkanShader->GetPipelineStageInfos())
			{
				auto& newStageInfo = pipelineStageInfos.emplace_back();
				newStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				newStageInfo.stage = Utility::VoltToVulkanShaderStage(stage);
				newStageInfo.module = stageInfo.shaderModule;
				newStageInfo.pName = "main"; // #TODO_Ivar: Set correct entry point
			}

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext = &pipelineRenderingInfo;
			pipelineInfo.stageCount = static_cast<uint32_t>(pipelineStageInfos.size());
			pipelineInfo.pStages = pipelineStageInfos.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
			pipelineInfo.pViewportState = &viewportInfo;
			pipelineInfo.pRasterizationState = &rasterizerInfo;
			pipelineInfo.pMultisampleState = &multiSampleInfo;
			pipelineInfo.pColorBlendState = &blendInfo;
			pipelineInfo.pDepthStencilState = &depthStencilInfo;
			pipelineInfo.pDynamicState = &dynamicStateInfo;
			pipelineInfo.pTessellationState = nullptr;
			pipelineInfo.layout = m_pipelineLayout;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		
			VT_VK_CHECK(vkCreateGraphicsPipelines(device->GetHandle<VkDevice>(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
		}
	}

	void* VulkanRenderPipeline::GetHandleImpl()
	{
		return m_pipeline;
	}

	void VulkanRenderPipeline::Release()
	{
		// #TODO_Ivar: Move to release queue
		if (m_pipeline == nullptr)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();
		vkDestroyPipelineLayout(device->GetHandle<VkDevice>(), m_pipelineLayout, nullptr);
		vkDestroyPipeline(device->GetHandle<VkDevice>(), m_pipeline, nullptr);

		m_pipelineLayout = nullptr;
		m_pipeline = nullptr;
	}
}
