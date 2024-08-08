#include "RenderContext.h"
#include "rcpch.h"
#include "RenderContext.h"

#include "RenderCore/RenderGraph/RenderGraphPass.h"
#include "RenderCore/RenderGraph/RenderGraph.h"
#include "RenderCore/Resources/BindlessResourcesManager.h"
#include "RenderCore/Debug/ShaderRuntimeValidator.h"

#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Descriptors/DescriptorTable.h>
#include <RHIModule/Images/Image3D.h>

#include <RHIModule/Graphics/GraphicsContext.h>

#include <RHIModule/Images/ImageView.h>

namespace Volt
{
	RenderContext::RenderContext(RefPtr<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	RenderContext::~RenderContext()
	{
	}

	void RenderContext::BeginMarker(std::string_view markerName, const glm::vec4& markerColor)
	{
		m_commandBuffer->BeginMarker(markerName, { markerColor.x, markerColor.y, markerColor.z, markerColor.w });
	}

	void RenderContext::EndMarker()
	{
		m_commandBuffer->EndMarker();
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

	const RenderingInfo RenderContext::CreateRenderingInfo(const uint32_t width, const uint32_t height, const StackVector<RenderGraphImage2DHandle, RHI::MAX_ATTACHMENT_COUNT>& attachments)
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

	void RenderContext::ClearImage(RenderGraphImage2DHandle handle, const glm::vec4& clearColor)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(handle);

		const auto image = m_renderGraph->GetImage2DRaw(handle);
		m_commandBuffer->ClearImage(image, { clearColor.x, clearColor.y, clearColor.z, clearColor.w });
	}

	void RenderContext::ClearImage(RenderGraphImage3DHandle handle, const glm::vec4& clearColor)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(handle);

		const auto image = m_renderGraph->GetImage3DRaw(handle);
		m_commandBuffer->ClearImage(image, { clearColor.x, clearColor.y, clearColor.z, clearColor.w });
	}

	void RenderContext::ClearBuffer(RenderGraphBufferHandle handle, uint32_t clearValue)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(handle);

		const auto buffer = m_renderGraph->GetBufferRaw(handle);
		m_commandBuffer->ClearBuffer(buffer, clearValue);
	}

	void RenderContext::CopyBuffer(RenderGraphBufferHandle src, RenderGraphBufferHandle dst, const size_t size)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(src);
		resourceAccess.ValidateResourceAccess(dst);

		const auto srcBuffer = m_renderGraph->GetBufferRaw(src);
		const auto dstBuffer = m_renderGraph->GetBufferRaw(dst);

		m_commandBuffer->CopyBufferRegion(srcBuffer->GetAllocation(), 0, dstBuffer->GetAllocation(), 0, size);
	}

	void RenderContext::MappedBufferUpload(RenderGraphBufferHandle bufferHandle, const void* data, const size_t size)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(bufferHandle);

		const auto buffer = m_renderGraph->GetBufferRaw(bufferHandle);
		uint8_t* mappedPtr = buffer->Map<uint8_t>();
		memcpy_s(mappedPtr, size, data, size);
		buffer->Unmap();
	}

	void RenderContext::PushConstants(const void* data, const uint32_t size)
	{
		BindDescriptorTableIfRequired();

		bool shouldPushConstants = false;

		if (m_currentRenderPipeline)
		{
			shouldPushConstants = m_currentRenderPipeline->GetShader()->HasConstants();
		}
		else if (m_currentComputePipeline)
		{
			shouldPushConstants = m_currentComputePipeline->GetShader()->HasConstants();
		}

		if (shouldPushConstants)
		{
			m_commandBuffer->PushConstants(data, size, 0);
		}
	}

	void RenderContext::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		m_commandBuffer->DispatchMeshTasks(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DispatchMeshTasksIndirect(RenderGraphBufferHandle commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		m_commandBuffer->DispatchMeshTasksIndirect(cmdsBuffer, offset, drawCount, stride);
	}

	void RenderContext::DispatchMeshTasksIndirectCount(RenderGraphBufferHandle commandsBuffer, const size_t offset, RenderGraphBufferHandle countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
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

	void RenderContext::DispatchIndirect(RenderGraphBufferHandle commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();
		ValidateCurrentPipelineConstants();

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		resourceAccess.ValidateResourceAccess(commandsBuffer);

		const auto cmdsBuffer = m_renderGraph->GetBufferRaw(commandsBuffer);
		m_commandBuffer->DispatchIndirect(cmdsBuffer, offset);
	}

	void RenderContext::DrawIndirectCount(RenderGraphBufferHandle commandsBuffer, const size_t offset, RenderGraphBufferHandle countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
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

	void RenderContext::DrawIndexedIndirect(RenderGraphBufferHandle commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
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

		InitializeCurrentPipelineConstantsValidation();
	}

	void RenderContext::BindIndexBuffer(RenderGraphBufferHandle indexBuffer)
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

	void RenderContext::BindVertexBuffers(const StackVector<WeakPtr<RHI::VertexBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		m_commandBuffer->BindVertexBuffers(vertexBuffers, firstBinding);
	}

	void RenderContext::BindVertexBuffers(const StackVector<RenderGraphBufferHandle, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding)
	{
		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		
		for (const auto& buffer : vertexBuffers)
		{
			resourceAccess.ValidateResourceAccess(buffer);
		}

		StackVector<WeakPtr<RHI::StorageBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT> buffers{};
		for (const auto& buffer : vertexBuffers)
		{
			buffers.EmplaceBack() = m_renderGraph->GetBufferRaw(buffer);
		}

		m_commandBuffer->BindVertexBuffers(buffers, firstBinding);
	}

	void RenderContext::Flush()
	{
		m_commandBuffer->End();

		m_commandBuffer->ExecuteAndWait();
		m_commandBuffer->RestartAfterFlush();
	}

	RefPtr<RHI::StorageBuffer> RenderContext::GetReadbackBuffer(WeakPtr<RHI::StorageBuffer> buffer)
	{
		RefPtr<RHI::StorageBuffer> readbackBuffer = RHI::StorageBuffer::Create(buffer->GetCount(), buffer->GetElementSize(), "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);

		RefPtr<RHI::CommandBuffer> tempCommandBuffer = RHI::CommandBuffer::Create();
		tempCommandBuffer->Begin();

		tempCommandBuffer->CopyBufferRegion(buffer->GetAllocation(), 0, readbackBuffer->GetAllocation(), 0, buffer->GetByteSize());

		tempCommandBuffer->End();
		tempCommandBuffer->ExecuteAndWait();

		return readbackBuffer;
	}

	void RenderContext::SetPerPassConstantsBuffer(WeakPtr<RHI::StorageBuffer> constantsBuffer)
	{
		m_perPassConstantsBuffer = constantsBuffer;
		m_perPassConstantsBufferData.resize(constantsBuffer->GetByteSize());
		memset(m_perPassConstantsBufferData.data(), 0, m_perPassConstantsBufferData.size());
	}

	void RenderContext::SetRenderGraphConstantsBuffer(WeakPtr<RHI::UniformBuffer> constantsBuffer)
	{
		m_renderGraphConstantsBuffer = constantsBuffer;
	}

	void RenderContext::SetCurrentPass(Weak<RenderGraphPassNodeBase> currentPassNode)
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
		uint8_t* mappedPtr = m_perPassConstantsBuffer->Map<uint8_t>();
		memcpy_s(mappedPtr, m_perPassConstantsBuffer->GetByteSize(), m_perPassConstantsBufferData.data(), m_perPassConstantsBufferData.size());
		m_perPassConstantsBuffer->Unmap();
	}

	void RenderContext::InitializeCurrentPipelineConstantsValidation()
	{
#ifdef VT_DEBUG
		VT_ASSERT_MSG(m_currentRenderPipeline || m_currentComputePipeline, "A pipeline must be bound!");
	
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
			VT_ASSERT_MSG(value, "All constants must have been set!");
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
		VT_ENSURE(m_currentComputePipeline || m_currentRenderPipeline);

		if (m_descriptorTableIsBound)
		{
			return;
		}

		// Set render graph constants
		{
			RenderGraphConstants renderGraphConstants;
			renderGraphConstants.constatsBufferIndex = BindlessResourcesManager::Get().GetBufferHandle(m_perPassConstantsBuffer);
			renderGraphConstants.constantsOffset = m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE;
#ifndef VT_DIST
			renderGraphConstants.shaderValidationBuffer = ShaderRuntimeValidator::Get().GetCurrentErrorBufferHandle();
#endif
			{
				uint8_t* constantsPtr = m_renderGraphConstantsBuffer->Map<uint8_t>(m_currentPassIndex);
				memcpy_s(constantsPtr, sizeof(RenderGraphConstants), &renderGraphConstants, sizeof(RenderGraphConstants));
				m_renderGraphConstantsBuffer->Unmap();
			}
		}

		auto descriptorTable = BindlessResourcesManager::Get().GetDescriptorTable();
		descriptorTable->SetConstantsBuffer(m_renderGraphConstantsBuffer);
		descriptorTable->SetOffsetIndexAndStride(m_currentPassIndex, sizeof(RenderGraphConstants));
		m_commandBuffer->BindDescriptorTable(descriptorTable);

		m_descriptorTableIsBound = true;
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
		VT_ENSURE(uniform.type.baseType == RHI::ShaderUniformBaseType::Sampler);
#endif

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &data, sizeof(ResourceHandle));
	}
	
	void RenderContext::SetConstant(const StringHash& name, const RenderGraphImage2DHandle& data, const int32_t mip, const int32_t layer)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;

		VT_ENSURE(uniform.type.baseType == RHI::ShaderUniformBaseType::Texture2D ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture2D ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::Texture2DArray ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture2DArray ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::TextureCube);

		if (uniform.type.baseType == RHI::ShaderUniformBaseType::Texture2D ||
				 uniform.type.baseType == RHI::ShaderUniformBaseType::Texture2DArray ||
				 uniform.type.baseType == RHI::ShaderUniformBaseType::TextureCube)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture2D ||
				 uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture2DArray)
		{
			VT_ENSURE(m_currentPassNode->WritesResource(data) || m_currentPassNode->CreatesResource(data));
		}
#endif

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		const ResourceHandle resourceHandle = resourceAccess.GetImage2D(data, mip, layer);

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &resourceHandle, sizeof(ResourceHandle));
	}

	void RenderContext::SetConstant(const StringHash& name, const RenderGraphImage3DHandle& data, const int32_t mip, const int32_t layer)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;

		VT_ENSURE(uniform.type.baseType == RHI::ShaderUniformBaseType::Texture3D ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture3D);

		if (uniform.type.baseType == RHI::ShaderUniformBaseType::Texture3D)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::RWTexture3D)
		{
			VT_ENSURE(m_currentPassNode->WritesResource(data) || m_currentPassNode->CreatesResource(data));
		}
#endif

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		const ResourceHandle resourceHandle = resourceAccess.GetImage3D(data, mip, layer);

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &resourceHandle, sizeof(ResourceHandle));
	}

	template<>
	void RenderContext::SetConstant(const StringHash& name, const RenderGraphBufferHandle& data)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;

		VT_ENSURE(uniform.type.baseType == RHI::ShaderUniformBaseType::Buffer ||
				uniform.type.baseType == RHI::ShaderUniformBaseType::RWBuffer);

		if (uniform.type.baseType == RHI::ShaderUniformBaseType::Buffer)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data) || m_currentPassNode->CreatesResource(data));
		}
		else if (uniform.type.baseType == RHI::ShaderUniformBaseType::RWBuffer)
		{
			VT_ENSURE(m_currentPassNode->WritesResource(data) || m_currentPassNode->CreatesResource(data));
		}
#endif

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		const ResourceHandle resourceHandle = resourceAccess.GetBuffer(data);

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &resourceHandle, sizeof(ResourceHandle));
	}

	template<>
	void RenderContext::SetConstant(const StringHash& name, const RenderGraphUniformBufferHandle& data)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;

		VT_ENSURE(uniform.type.baseType == RHI::ShaderUniformBaseType::UniformBuffer);

		if (uniform.type.baseType == RHI::ShaderUniformBaseType::UniformBuffer)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data));
		}
#endif

		RenderGraphPassResources resourceAccess{ *m_renderGraph, *m_currentPassNode };
		const ResourceHandle resourceHandle = resourceAccess.GetUniformBuffer(data);

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &resourceHandle, sizeof(ResourceHandle));
	}
}
