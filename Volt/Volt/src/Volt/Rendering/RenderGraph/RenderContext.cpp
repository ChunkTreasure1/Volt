#include "RenderContext.h"
#include "vtpch.h"
#include "RenderContext.h"

#include "Volt/Rendering/RenderGraph/RenderGraphPass.h"
#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/Debug/ShaderRuntimeValidator.h"
#include "Volt/Rendering/Renderer.h"

#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <VoltRHI/Images/ImageView.h>

namespace Volt
{
	RenderContext::RenderContext(RefPtr<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	RenderContext::~RenderContext()
	{
	}

	void RenderContext::BeginRendering(const RenderingInfo& renderingInfo)
	{
		VT_PROFILE_FUNCTION();

		m_commandBuffer->SetViewports({ renderingInfo.viewport });
		m_commandBuffer->SetScissors({ renderingInfo.scissor });
		m_commandBuffer->BeginRendering(renderingInfo.renderingInfo);
	}

	void RenderContext::EndRendering()
	{
		VT_PROFILE_FUNCTION();

		m_commandBuffer->EndRendering();
	}

	const RenderingInfo RenderContext::CreateRenderingInfo(const uint32_t width, const uint32_t height, const StackVector<RenderGraphResourceHandle, RHI::MAX_ATTACHMENT_COUNT>& attachments)
	{
		RHI::Rect2D scissor = { 0, 0, width, height };
		RHI::Viewport viewport{};
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		StackVector<RHI::AttachmentInfo, RHI::MAX_COLOR_ATTACHMENT_COUNT> colorAttachments;
		RHI::AttachmentInfo depthAttachment{};

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };

		for (const auto& resourceHandle : attachments)
		{
			resourceAccess.ValidateResourceAccess(resourceHandle);
			const auto view = m_renderGraph->GetImage2DView(resourceHandle);

			if ((view->GetImageAspect() & RHI::ImageAspect::Color) != RHI::ImageAspect::None)
			{
				RHI::AttachmentInfo attachment{};
				attachment.clearMode = RHI::ClearMode::Clear;
				attachment.clearColor = { 0.f, 0.f, 0.f, 0.f };
				attachment.view = view;

				colorAttachments.Push(attachment);
			}
			else
			{
				depthAttachment.clearMode = RHI::ClearMode::Clear;
				depthAttachment.clearColor = { 0.f };
				depthAttachment.view = view;
			}
		}

		RHI::RenderingInfo renderingInfo{};
		renderingInfo.colorAttachments = colorAttachments;
		renderingInfo.depthAttachmentInfo = depthAttachment;
		renderingInfo.renderArea = scissor;

		RenderingInfo result{};
		result.renderingInfo = renderingInfo;
		result.scissor = scissor;
		result.viewport = viewport;

		return result;
	}

	void RenderContext::ClearImage(RenderGraphResourceHandle handle, const glm::vec4& clearColor)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(handle);

		const auto image = m_renderGraph->GetImage2DRaw(handle);
		m_commandBuffer->ClearImage(image, { clearColor.x, clearColor.y, clearColor.z, clearColor.w });
	}

	void RenderContext::ClearBuffer(RenderGraphResourceHandle handle, uint32_t clearValue)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(handle);

		const auto buffer = m_renderGraph->GetBufferRaw(handle);
		m_commandBuffer->ClearBuffer(buffer, clearValue);
	}

	void RenderContext::UploadBufferData(RenderGraphResourceHandle bufferHandle, const void* data, const size_t size)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(bufferHandle);

		const auto buffer = m_renderGraph->GetBufferRaw(bufferHandle);
		buffer->SetData(m_commandBuffer, data, size);
	}

	void RenderContext::MappedBufferUpload(RenderGraphResourceHandle bufferHandle, const void* data, const size_t size)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(bufferHandle);

		const auto buffer = m_renderGraph->GetBufferRaw(bufferHandle);
		uint8_t* mappedPtr = buffer->Map<uint8_t>();
		memcpy_s(mappedPtr, size, data, size);
		buffer->Unmap();
	}

	void RenderContext::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		m_commandBuffer->DispatchMeshTasks(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DispatchMeshTasksIndirect(RenderGraphResourceHandle commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		m_commandBuffer->DispatchMeshTasksIndirect(cmdsBuffer, offset, drawCount, stride);
	}

	void RenderContext::DispatchMeshTasksIndirectCount(RenderGraphResourceHandle commandsBuffer, const size_t offset, RenderGraphResourceHandle countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);
		resourceAccess.ValidateResourceAccess(countBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		const auto cntsBuffer = m_renderGraph->GetBufferRaw(countBuffer);

		m_commandBuffer->DispatchMeshTasksIndirectCount(cmdsBuffer, offset, cntsBuffer, countBufferOffset, maxDrawCount, stride);
	}

	void RenderContext::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		m_commandBuffer->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DispatchIndirect(RenderGraphResourceHandle commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		m_commandBuffer->DispatchIndirect(cmdsBuffer, offset);
	}

	void RenderContext::DrawIndirectCount(RenderGraphResourceHandle commandsBuffer, const size_t offset, RenderGraphResourceHandle countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		if (maxDrawCount == 0)
		{
			return;
		}

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);
		resourceAccess.ValidateResourceAccess(countBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		const auto cntsBuffer = m_renderGraph->GetBufferRaw(countBuffer);
		m_commandBuffer->DrawIndirectCount(cmdsBuffer, offset, cntsBuffer, countBufferOffset, maxDrawCount, stride);
	}

	void RenderContext::DrawIndexedIndirect(RenderGraphResourceHandle commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		if (drawCount == 0)
		{
			return;
		}

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		m_commandBuffer->DrawIndexedIndirect(cmdsBuffer, offset, drawCount, stride);
	}

	void RenderContext::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		m_commandBuffer->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void RenderContext::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		m_commandBuffer->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void RenderContext::BindPipeline(WeakPtr<RHI::RenderPipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentComputePipeline.Reset();

		if (!pipeline)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;
		m_commandBuffer->BindPipeline(pipeline);

		m_descriptorTableIsBound = false;
		m_currentDescriptorTable = GetOrCreateDescriptorTable(pipeline);
	
		InitializeCurrentPipelineConstantsValidation();
	}

	void RenderContext::BindPipeline(WeakPtr<RHI::ComputePipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentRenderPipeline.Reset();

		if (!pipeline)
		{
			m_currentComputePipeline.Reset();
			return;
		}

		m_currentComputePipeline = pipeline;
		m_commandBuffer->BindPipeline(pipeline);

		m_descriptorTableIsBound = false;
		m_currentDescriptorTable = GetOrCreateDescriptorTable(pipeline);

		InitializeCurrentPipelineConstantsValidation();
	}

	void RenderContext::BindIndexBuffer(RenderGraphResourceHandle indexBuffer)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(indexBuffer);

		const auto idxBuffer = m_renderGraph->GetBufferRaw(indexBuffer);
		m_commandBuffer->BindIndexBuffer(idxBuffer);
	}

	void RenderContext::BindIndexBuffer(WeakPtr<RHI::IndexBuffer> indexBuffer)
	{
		m_commandBuffer->BindIndexBuffer(indexBuffer);
	}

	void RenderContext::BindVertexBuffers(const std::vector<WeakPtr<RHI::VertexBuffer>>& vertexBuffers, const uint32_t firstBinding)
	{
		m_commandBuffer->BindVertexBuffers(vertexBuffers, firstBinding);
	}

	void RenderContext::Flush()
	{
		m_commandBuffer->End();

		m_commandBuffer->ExecuteAndWait();
		m_commandBuffer->RestartAfterFlush();
	}

	RefPtr<RHI::StorageBuffer> RenderContext::GetReadbackBuffer(WeakPtr<RHI::StorageBuffer> buffer)
	{
		RefPtr<RHI::StorageBuffer> readbackBuffer = RHI::StorageBuffer::Create(buffer->GetSize(), "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);

		RefPtr<RHI::CommandBuffer> tempCommandBuffer = RHI::CommandBuffer::Create();
		tempCommandBuffer->Begin();

		tempCommandBuffer->CopyBufferRegion(buffer->GetAllocation(), 0, readbackBuffer->GetAllocation(), 0, buffer->GetSize());

		tempCommandBuffer->End();
		tempCommandBuffer->ExecuteAndWait();

		return readbackBuffer;
	}

	void RenderContext::SetPassConstantsBuffer(WeakPtr<RHI::StorageBuffer> constantsBuffer)
	{
		m_passConstantsBuffer = constantsBuffer;
		m_passConstantsBufferData.resize(constantsBuffer->GetSize());
		memset(m_passConstantsBufferData.data(), 0, m_passConstantsBufferData.size());
	}

	void RenderContext::SetCurrentPassIndex(Weak<RenderGraphPassNodeBase> currentPassNode)
	{
		m_currentPassIndex = currentPassNode->index;
		m_currentPassNode = currentPassNode;
	}

	void RenderContext::SetRenderGraphInstance(RenderGraph* renderGraph)
	{
		m_renderGraph = renderGraph;
	}

	void RenderContext::UploadConstantsData()
	{
		uint8_t* mappedPtr = m_passConstantsBuffer->Map<uint8_t>();
		memcpy_s(mappedPtr, m_passConstantsBuffer->GetSize(), m_passConstantsBufferData.data(), m_passConstantsBufferData.size());
		m_passConstantsBuffer->Unmap();
	}

	void RenderContext::InitializeCurrentPipelineConstantsValidation()
	{
#ifdef VT_DEBUG
		VT_CORE_ASSERT(m_currentRenderPipeline || m_currentComputePipeline, "A pipeline must be bound!");
	
		m_boundPipelineData.uniformHasBeenSetMap.clear();

		const auto& currentConstants = GetRenderGraphConstantsData();
		for (const auto& constant : currentConstants.uniforms)
		{
			m_boundPipelineData.uniformHasBeenSetMap[constant.first] = false;
		}
#endif
	}

	void RenderContext::ValidateCurrentPipelineConstants()
	{
#ifdef VT_DEBUG

		for (const auto& [hash, value] : m_boundPipelineData.uniformHasBeenSetMap)
		{
			VT_CORE_ASSERT(value, "All constants must have been set!");
		}
#endif
	}

	const RHI::ShaderRenderGraphConstantsData& RenderContext::GetRenderGraphConstantsData()
	{
		if (m_currentRenderPipeline)
		{
			return m_currentRenderPipeline->GetShader()->GetResources().renderGraphConstantsData;
		}
		else
		{
			return m_currentComputePipeline->GetShader()->GetResources().renderGraphConstantsData;
		}
	}

	void RenderContext::BindDescriptorTableIfRequired()
	{
		VT_PROFILE_FUNCTION();

		if (m_descriptorTableIsBound)
		{
			return;
		}

		struct PushConstantsData
		{
			ResourceHandle constatsBufferIndex;
			ResourceHandle shaderValidationBuffer;
			uint32_t constantsOffset;
		} constantsData;

		constantsData.constatsBufferIndex = BindlessResourcesManager::Get().GetBufferHandle(m_passConstantsBuffer);
		
#ifndef VT_DIST
		constantsData.shaderValidationBuffer = Renderer::GetRuntimeShaderValidator().GetCurrentErrorBufferHandle();
#endif
		
		constantsData.constantsOffset = m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE;

		m_commandBuffer->PushConstants(&constantsData, sizeof(PushConstantsData), 0);
		m_commandBuffer->BindDescriptorTable(m_currentDescriptorTable);

		m_descriptorTableIsBound = true;
	}

	RefPtr<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(WeakPtr<RHI::RenderPipeline> renderPipeline)
	{
		VT_PROFILE_FUNCTION();

		//auto shader = renderPipeline->GetShader();
		//void* ptr = shader.get();

		//if (m_descriptorTableCache.contains(ptr))
		//{
		//	return m_descriptorTableCache.at(ptr);
		//}

		//RHI::DescriptorTableCreateInfo info{};
		//info.shader = shader;
		//info.count = 1;

		//RefPtr<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		//m_descriptorTableCache[ptr] = descriptorTable;

		return BindlessResourcesManager::Get().GetDescriptorTable();
	}

	RefPtr<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(WeakPtr<RHI::ComputePipeline> computePipeline)
	{
		VT_PROFILE_FUNCTION();

		//auto shader = computePipeline->GetShader();
		//void* ptr = shader.get();

		//if (m_descriptorTableCache.contains(ptr))
		//{
		//	return m_descriptorTableCache.at(ptr);
		//}

		//RHI::DescriptorTableCreateInfo info{};
		//info.shader = shader;
		//info.count = 1;

		//RefPtr<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		//m_descriptorTableCache[ptr] = descriptorTable;

		return BindlessResourcesManager::Get().GetDescriptorTable();
	}

	template<>
	void RenderContext::SetConstant(const StringHash& name, const ResourceHandle& data)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;

		VT_ENSURE(uniform.type.baseType == RHI::ShaderUniformBaseType::Buffer ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::RWBuffer ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::Texture ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::UniformBuffer ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::Sampler);

		if (uniform.type.baseType == RHI::ShaderUniformBaseType::Buffer)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data) || m_currentPassNode->CreatesResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::RWBuffer)
		{
			VT_ENSURE(m_currentPassNode->WritesResource(data) || m_currentPassNode->CreatesResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::Texture)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture)
		{
			VT_ENSURE(m_currentPassNode->WritesResource(data) || m_currentPassNode->CreatesResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::UniformBuffer)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data));
		}
		//else if (uniform.type.baseType == RHI::ShaderUniformBaseType::Sampler) // #TODO_Ivar: Should we validate samplers?
		//{
		//	VT_ENSURE(m_currentPassNode->ReadsResource(data));
		//}
#endif

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &data, sizeof(ResourceHandle));
	}
}
