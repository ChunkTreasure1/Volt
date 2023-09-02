#include "vtpch.h"
#include "RenderPipeline.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"
#include "Volt/Rendering/VulkanFramebuffer.h"
#include "Volt/Rendering/VulkanRenderPass.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/GlobalDescriptorSetManager.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	namespace Utility
	{
		static VkPrimitiveTopology VoltToVulkanTopology(Topology topology)
		{
			switch (topology)
			{
				case Topology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				case Topology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				case Topology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				case Topology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
				case Topology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

			}

			VT_CORE_ASSERT(false, "Topology not supported!");
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		static VkPolygonMode VoltToVulkanFill(FillMode fillMode)
		{
			switch (fillMode)
			{
				case FillMode::Solid: return VK_POLYGON_MODE_FILL;
				case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
			}

			VT_CORE_ASSERT(false, "Fill mode not supported!");
			return VK_POLYGON_MODE_FILL;
		}

		static VkCullModeFlags VoltToVulkanCull(CullMode cullMode)
		{
			switch (cullMode)
			{
				case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
				case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
				case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
				case CullMode::None: return VK_CULL_MODE_NONE;
			}

			VT_CORE_ASSERT(false, "Cull mode not supported!");
			return VK_CULL_MODE_BACK_BIT;
		}

		static VkFormat VoltToVulkanElementFormat(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return VK_FORMAT_R8_UINT;
				
				case ElementType::Byte: return VK_FORMAT_R8_UINT;
				case ElementType::Byte2: return VK_FORMAT_R8G8_UINT;
				case ElementType::Byte3: return VK_FORMAT_R8G8B8_UINT;
				case ElementType::Byte4: return VK_FORMAT_R8G8B8A8_UINT;

				case ElementType::Half: return VK_FORMAT_R16_SFLOAT;
				case ElementType::Half2: return VK_FORMAT_R16G16_SFLOAT;
				case ElementType::Half3: return VK_FORMAT_R16G16B16_SFLOAT;
				case ElementType::Half4: return VK_FORMAT_R16G16B16A16_SFLOAT;

				case ElementType::UShort: return VK_FORMAT_R16_UINT;
				case ElementType::UShort2: return VK_FORMAT_R16G16_UINT;
				case ElementType::UShort3: return VK_FORMAT_R16G16B16_UINT;
				case ElementType::UShort4: return VK_FORMAT_R16G16B16A16_UINT;

				case ElementType::Int: return VK_FORMAT_R32_SINT;
				case ElementType::Int2: return VK_FORMAT_R32G32_SINT;
				case ElementType::Int3: return VK_FORMAT_R32G32B32_SINT;
				case ElementType::Int4: return VK_FORMAT_R32G32B32A32_SINT;
				
				case ElementType::UInt: return VK_FORMAT_R32_UINT;
				case ElementType::UInt2: return VK_FORMAT_R32G32_UINT;
				case ElementType::UInt3: return VK_FORMAT_R32G32B32_UINT;
				case ElementType::UInt4: return VK_FORMAT_R32G32B32A32_UINT;
				
				case ElementType::Float: return VK_FORMAT_R32_SFLOAT;
				case ElementType::Float2: return VK_FORMAT_R32G32_SFLOAT;
				case ElementType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ElementType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
				
				case ElementType::Float3x3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ElementType::Float4x4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			return VK_FORMAT_R8G8B8A8_UNORM;
		}
	}

	RenderPipeline::RenderPipeline(const RenderPipelineSpecification& specification)
		: mySpecification(specification)
	{
		VT_CORE_ASSERT(specification.shader, "Shader must be specified!");

		Invalidate();

		Renderer::AddShaderDependency(specification.shader, this);
	}

	RenderPipeline::RenderPipeline(const RenderPipelineSpecification& specification, const std::map<ShaderStage, ShaderDataBuffer>& specializationConstants, size_t permutationHash)
		: mySpecification(specification), mySpecializationConstantsBuffers(specializationConstants), myIsPermutation(true), myPermutationHash(permutationHash)
	{
		VT_CORE_ASSERT(specification.shader, "Shader must be specified!");
		Invalidate(specializationConstants);

		Renderer::AddShaderDependency(specification.shader, this);
	}

	RenderPipeline::~RenderPipeline()
	{
		if (mySpecification.shader)
		{
			Renderer::RemoveShaderDependency(mySpecification.shader, this);
		}
		Release();
	}

	void RenderPipeline::Invalidate(const std::map<ShaderStage, ShaderDataBuffer>& definedConstants)
	{
		if (!definedConstants.empty())
		{
			mySpecializationConstantsBuffers = definedConstants;
		}

		Release();

		if (mySpecification.vertexLayout.IsValid() && mySpecification.shader->GetResources().vertexLayout.IsValid())
		{
			CreateVertexLayout();
		}

		auto resources = mySpecification.shader->GetResources();
		//auto device = GraphicsContextVolt::GetDevice();

		// Pipeline Layout
		{
			// Find global descriptor sets
			for (uint32_t set = 0; set < static_cast<uint32_t>(resources.nullPaddedDescriptorSetLayouts.size()); set++)
			{
				if (GlobalDescriptorSetManager::HasDescriptorSet(set))
				{
					const auto descriptorSetLayout = GlobalDescriptorSetManager::GetDescriptorSet(set)->GetDescriptorSetLayout();
					resources.nullPaddedDescriptorSetLayouts[set] = descriptorSetLayout;
				}
			}

			VkPipelineLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.setLayoutCount = static_cast<uint32_t>(resources.nullPaddedDescriptorSetLayouts.size());
			info.pSetLayouts = resources.nullPaddedDescriptorSetLayouts.data();
			info.pushConstantRangeCount = resources.pushConstantRange.size > 0 ? 1 : 0;
			info.pPushConstantRanges = &resources.pushConstantRange;

			VT_CORE_ASSERT(resources.pushConstantRange.size <= 128, "Push constant range must be less or equal to 128 bytes to support all platforms!");
			//VT_VK_CHECK(vkCreatePipelineLayout(device->GetHandle(), &info, nullptr, &myPipelineLayout));
		}

		// Pipeline
		{
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)myVertexBindingDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = myVertexBindingDescriptions.data();

			vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)myVertexAttributeDescriptions.size();
			vertexInputInfo.pVertexAttributeDescriptions = myVertexAttributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.topology = Utility::VoltToVulkanTopology(mySpecification.topology);
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
			rasterizerInfo.polygonMode = Utility::VoltToVulkanFill(mySpecification.fillMode);
			rasterizerInfo.cullMode = Utility::VoltToVulkanCull(mySpecification.cullMode);
			rasterizerInfo.lineWidth = mySpecification.lineWidth;
			rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizerInfo.depthBiasEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tessellationInfo{};
			tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tessellationInfo.patchControlPoints = mySpecification.tessellationControlPoints;

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

			const bool hasRenderPass = mySpecification.renderPass;

			std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{};
			std::vector<FramebufferAttachment> attachments;

			if (hasRenderPass)
			{
				attachments = mySpecification.renderPass->framebuffer->GetSpecification().attachments;
			}
			else
			{
				attachments = mySpecification.framebufferAttachments;
			}

			for (const auto& attachment : attachments)
			{
				if (Utility::IsDepthFormat(attachment.format))
				{
					continue;
				}

				VkPipelineColorBlendAttachmentState& blendAttachment = blendAttachments.emplace_back();
				blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				blendAttachment.blendEnable = attachment.blending != TextureBlend::None ? VK_TRUE : VK_FALSE;

				switch (attachment.blending)
				{
					case TextureBlend::Add:
					{
						blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
						blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
						blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
						blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
						blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
						break;
					}

					case TextureBlend::Alpha:
					{
						blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
						blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
						blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
						blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
						blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
						break;
					}

					case TextureBlend::ZeroSrcColor:
					{
						blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
						blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

						blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
						blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
						break;
					}

					case TextureBlend::OneMinusSrcColor:
					{
						blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
						blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

						blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
						blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

						break;
					}

					case TextureBlend::OneMinusSrcAlpha:
					{
						blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
						blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
						blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

						blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
						blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

						break;
					}
				}
			}

			blendInfo.attachmentCount = (uint32_t)blendAttachments.size();
			blendInfo.pAttachments = blendAttachments.data();

			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
			depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilInfo.depthCompareOp = Utility::VoltToVulkanCompareOp(mySpecification.depthCompareOperator);
			depthStencilInfo.stencilTestEnable = VK_FALSE;
			depthStencilInfo.depthBoundsTestEnable = VK_FALSE;

			switch (mySpecification.depthMode)
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
				VK_DYNAMIC_STATE_LINE_WIDTH
			};

			dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
			dynamicStateInfo.pDynamicStates = dynamicStates.data();

			VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
			pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

			std::vector<VkFormat> attachmentFormats{};
			VkFormat depthFormat = VK_FORMAT_UNDEFINED;
			VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

			if (hasRenderPass)
			{
				attachmentFormats = mySpecification.renderPass->framebuffer->GetColorFormats();
				depthFormat = mySpecification.renderPass->framebuffer->GetDepthFormat();
				if (Utility::IsStencilFormat(mySpecification.renderPass->framebuffer->GetDepthAttachment()->GetFormat()))
				{
					stencilFormat = mySpecification.renderPass->framebuffer->GetDepthFormat();
				}
			}
			else
			{

				for (const auto& att : attachments)
				{
					if (!Utility::IsDepthFormat(att.format))
					{
						attachmentFormats.emplace_back(Utility::VoltToVulkanFormat(att.format));
					}
					else
					{
						depthFormat = Utility::VoltToVulkanFormat(att.format);
						if (Utility::IsStencilFormat(att.format))
						{
							stencilFormat = depthFormat;
						}
					}
				}
			}

			pipelineRenderingInfo.colorAttachmentCount = (uint32_t)attachmentFormats.size();
			pipelineRenderingInfo.pColorAttachmentFormats = attachmentFormats.data();

			pipelineRenderingInfo.depthAttachmentFormat = depthFormat;
			pipelineRenderingInfo.stencilAttachmentFormat = stencilFormat;

			auto stageInfos = mySpecification.shader->GetStageInfos();

			std::vector<VkSpecializationInfo> specConstInfos{};
			std::vector<std::vector<VkSpecializationMapEntry>> specConstEntries{};

			const auto specializationConstants = mySpecification.shader->GetResources().specializationConstants;

			if (myIsPermutation)
			{
				specConstInfos.reserve(stageInfos.size());
				for (auto& stageInfo : stageInfos)
				{
					if (!specializationConstants.contains(stageInfo.stage))
					{
						continue;
					}

					VT_CORE_ASSERT(mySpecializationConstantsBuffers.contains((ShaderStage)stageInfo.stage), "Invalid specialization constant buffers!");

					const auto& specConsts = specializationConstants.at(stageInfo.stage);
					auto& buffer = mySpecializationConstantsBuffers.at((ShaderStage)stageInfo.stage);

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
					stageInfo.pSpecializationInfo = &specConstInfo;
				}
			}

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext = &pipelineRenderingInfo;
			pipelineInfo.stageCount = (uint32_t)stageInfos.size();
			pipelineInfo.pStages = stageInfos.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
			pipelineInfo.pViewportState = &viewportInfo;
			pipelineInfo.pRasterizationState = &rasterizerInfo;
			pipelineInfo.pMultisampleState = &multiSampleInfo;
			pipelineInfo.pColorBlendState = &blendInfo;
			pipelineInfo.pDepthStencilState = &depthStencilInfo;
			pipelineInfo.pDynamicState = &dynamicStateInfo;
			pipelineInfo.pTessellationState = mySpecification.topology == Topology::PatchList ? &tessellationInfo : nullptr;
			pipelineInfo.layout = myPipelineLayout;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			//VT_VK_CHECK(vkCreateGraphicsPipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myPipeline));

			GenerateHash();
		}
	}

	void RenderPipeline::Recreate(const RenderPipelineSpecification& specification)
	{
		mySpecification = specification;
		Invalidate(mySpecializationConstantsBuffers);
	}

	void RenderPipeline::Bind(VkCommandBuffer commandBuffer) const
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipeline);
	}

	void RenderPipeline::BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayout, set, 1, &descriptorSet, 0, nullptr);
	}

	void RenderPipeline::PushConstants(VkCommandBuffer commandBuffer, const void* data, const size_t size) const
	{
		const auto stageFlags = mySpecification.shader->GetResources().pushConstantRange.stageFlags;
		vkCmdPushConstants(commandBuffer, myPipelineLayout, stageFlags, 0, (uint32_t)size, data);
	}

	bool RenderPipeline::HasDescriptorSet(uint32_t set) const
	{
		const auto& resources = mySpecification.shader->GetResources();
		return resources.descriptorSetBindings.contains(set); // Descriptor set bindings contains all the sets in the shader
	}

	void RenderPipeline::SetAttachments(const std::vector<FramebufferAttachment>& attachments)
	{
		mySpecification.framebufferAttachments = attachments;
	}

	const bool RenderPipeline::IsPermutationOf(Ref<RenderPipeline> renderPipeline) const
	{
		return myPermutationHash == renderPipeline->GetHash();
	}

	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineSpecification& specification)
	{
		return CreateRef<RenderPipeline>(specification);
	}

	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineSpecification& specification, const std::map<ShaderStage, ShaderDataBuffer>& specializationConstants, size_t permutationHash)
	{
		return CreateRef<RenderPipeline>(specification, specializationConstants, permutationHash);
	}

	void RenderPipeline::Release()
	{
		//Renderer::SubmitResourceChange([pipeline = myPipeline, pipelineLayout = myPipelineLayout]()
		//{
		//	if (pipeline != VK_NULL_HANDLE)
		//	{
		//		auto device = GraphicsContextVolt::GetDevice();
		//		vkDestroyPipelineLayout(device->GetHandle(), pipelineLayout, nullptr);
		//		vkDestroyPipeline(device->GetHandle(), pipeline, nullptr);
		//	}
		//});
	}

	void RenderPipeline::CreateVertexLayout()
	{
		VT_CORE_ASSERT(!mySpecification.vertexLayout.GetElements().empty(), "To create a pipeline a vertex layout must be specified!");

		myVertexAttributeDescriptions.clear();
		myVertexBindingDescriptions.clear();

		VkVertexInputBindingDescription& bindingDesc = myVertexBindingDescriptions.emplace_back();
		bindingDesc.binding = 0;
		bindingDesc.stride = mySpecification.vertexLayout.GetStride();
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		uint32_t attributeIndex = 0;
		for (const auto& element : mySpecification.vertexLayout.GetElements())
		{
			VkVertexInputAttributeDescription& desc = myVertexAttributeDescriptions.emplace_back();
			desc.binding = 0;
			desc.location = attributeIndex;
			desc.format = Utility::VoltToVulkanElementFormat(element.type);
			desc.offset = (uint32_t)element.offset;

			attributeIndex++;
		}

		if (!mySpecification.instanceLayout.GetElements().empty())
		{
			VkVertexInputBindingDescription& inputBindingDesc = myVertexBindingDescriptions.emplace_back();
			inputBindingDesc.binding = 1;
			inputBindingDesc.stride = mySpecification.instanceLayout.GetStride();
			inputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			for (const auto& element : mySpecification.instanceLayout.GetElements())
			{
				VkVertexInputAttributeDescription& desc = myVertexAttributeDescriptions.emplace_back();
				desc.binding = 1;
				desc.location = attributeIndex;
				desc.format = Utility::VoltToVulkanElementFormat(element.type);
				desc.offset = (uint32_t)element.offset;

				attributeIndex++;
			}
		}
	}

	void RenderPipeline::GenerateHash()
	{
		size_t hash = mySpecification.shader->GetHash();
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()((uint32_t)mySpecification.topology));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()((uint32_t)mySpecification.cullMode));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()((uint32_t)mySpecification.fillMode));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()((uint32_t)mySpecification.depthMode));
		hash = Utility::HashCombine(hash, std::hash<std::string>()(mySpecification.name));

		if (myIsPermutation)
		{
			hash = Utility::HashCombine(hash, std::hash<bool>()(myIsPermutation));

			for (const auto& [stage, buffer] : mySpecializationConstantsBuffers)
			{
				hash = Utility::HashCombine(hash, std::hash<const void*>()(buffer.GetData()));
			}
		}

		myHash = hash;
	}
}
