#include "RenderContext.h"
#include "vtpch.h"
#include "RenderContext.h"

#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"

#include "Volt/Core/Profiling.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>

#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <VoltRHI/Images/ImageView.h>

namespace Volt
{
	RenderContext::RenderContext(Ref<RHI::CommandBuffer> commandBuffer)
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

	const RenderingInfo RenderContext::CreateRenderingInfo(const uint32_t width, const uint32_t height, const std::vector<Ref<RHI::ImageView>>& attachments)
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

		for (const auto& view : attachments)
		{
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

	void RenderContext::ClearImage(Ref<RHI::Image2D> image, const glm::vec4& clearColor)
	{
		VT_PROFILE_FUNCTION();
		m_commandBuffer->ClearImage(image, { clearColor.x, clearColor.y, clearColor.z, clearColor.w });
	}

	void RenderContext::ClearBuffer(Ref<RHI::StorageBuffer> buffer, uint32_t clearValue)
	{
		m_commandBuffer->ClearBuffer(buffer, clearValue);
	}

	void RenderContext::UploadBufferData(Ref<RHI::StorageBuffer> buffer, const void* data, const size_t size)
	{
		buffer->SetData(m_commandBuffer, data, size);
	}

	void RenderContext::DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		BindDescriptorTableIfRequired();
		m_commandBuffer->DispatchMeshTasks(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DispatchMeshTasksIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();
		m_commandBuffer->DispatchMeshTasksIndirect(commandsBuffer, offset, drawCount, stride);
	}

	void RenderContext::DispatchMeshTasksIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();
		m_commandBuffer->DispatchMeshTasksIndirectCount(commandsBuffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
	}

	void RenderContext::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();

		m_commandBuffer->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DispatchIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();
		m_commandBuffer->DispatchIndirect(commandsBuffer, offset);
	}

	void RenderContext::DrawIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();
		BindDescriptorTableIfRequired();

		if (maxDrawCount == 0)
		{
			return;
		}

		m_commandBuffer->DrawIndirectCount(commandsBuffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
	}

	void RenderContext::DrawIndexedIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride)
	{
		BindDescriptorTableIfRequired();

		if (drawCount == 0)
		{
			return;
		}

		m_commandBuffer->DrawIndexedIndirect(commandsBuffer, offset, drawCount, stride);
	}

	void RenderContext::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		m_commandBuffer->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void RenderContext::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		m_commandBuffer->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void RenderContext::BindPipeline(Ref<RHI::RenderPipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentComputePipeline.Reset();

		if (pipeline == nullptr)
		{
			m_currentRenderPipeline.Reset();
			return;
		}

		m_currentRenderPipeline = pipeline;
		m_commandBuffer->BindPipeline(pipeline);

		m_descriptorTableIsBound = false;
		m_currentDescriptorTable = GetOrCreateDescriptorTable(pipeline);
	}

	void RenderContext::BindPipeline(Ref<RHI::ComputePipeline> pipeline)
	{
		VT_PROFILE_FUNCTION();

		m_currentRenderPipeline.Reset();

		if (pipeline == nullptr)
		{
			m_currentComputePipeline.Reset();
			return;
		}

		m_currentComputePipeline = pipeline;
		m_commandBuffer->BindPipeline(pipeline);

		m_descriptorTableIsBound = false;
		m_currentDescriptorTable = GetOrCreateDescriptorTable(pipeline);
	}

	void RenderContext::BindIndexBuffer(Ref<RHI::StorageBuffer> indexBuffer)
	{
		m_commandBuffer->BindIndexBuffer(indexBuffer);
	}

	void RenderContext::BindIndexBuffer(Ref<RHI::IndexBuffer> indexBuffer)
	{
		m_commandBuffer->BindIndexBuffer(indexBuffer);
	}

	void RenderContext::BindVertexBuffers(const std::vector<Ref<RHI::VertexBuffer>>& vertexBuffers, const uint32_t firstBinding)
	{
		m_commandBuffer->BindVertexBuffers(vertexBuffers, firstBinding);
	}

	void RenderContext::Flush()
	{
		m_commandBuffer->End();

		GlobalResourceManager::Update();
	
		m_commandBuffer->ExecuteAndWait();
		m_commandBuffer->RestartAfterFlush();
	}

	Ref<RHI::StorageBuffer> RenderContext::GetReadbackBuffer(Ref<RHI::StorageBuffer> buffer)
	{
		Ref<RHI::StorageBuffer> readbackBuffer = RHI::StorageBuffer::Create(buffer->GetSize(), "Readback Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);

		Ref<RHI::CommandBuffer> tempCommandBuffer = RHI::CommandBuffer::Create();
		tempCommandBuffer->Begin();

		tempCommandBuffer->CopyBufferRegion(buffer->GetAllocation(), 0, readbackBuffer->GetAllocation(), 0, buffer->GetSize());

		tempCommandBuffer->End();
		tempCommandBuffer->ExecuteAndWait();

		return readbackBuffer;
	}

	void RenderContext::SetPassConstantsBuffer(Weak<RHI::StorageBuffer> constantsBuffer)
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

	void RenderContext::UploadConstantsData()
	{
		uint8_t* mappedPtr = m_passConstantsBuffer->Map<uint8_t>();
		memcpy_s(mappedPtr, m_passConstantsBuffer->GetSize(), m_passConstantsBufferData.data(), m_passConstantsBufferData.size());
		m_passConstantsBuffer->Unmap();
	}

	RHI::ShaderRenderGraphConstantsData RenderContext::GetRenderGraphConstantsData()
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
			uint32_t constantsOffset;
		} constantsData;

		constantsData.constatsBufferIndex = GlobalResourceManager::GetResourceHandle(m_passConstantsBuffer);
		constantsData.constantsOffset = m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE;

		m_commandBuffer->PushConstants(&constantsData, sizeof(PushConstantsData), 0);
		m_commandBuffer->BindDescriptorTable(m_currentDescriptorTable);

		m_descriptorTableIsBound = true;
	}

	Ref<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(Ref<RHI::RenderPipeline> renderPipeline)
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

		//Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		//m_descriptorTableCache[ptr] = descriptorTable;

		return GlobalResourceManager::GetDescriptorTable();
	}

	Ref<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(Ref<RHI::ComputePipeline> computePipeline)
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

		//Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		//m_descriptorTableCache[ptr] = descriptorTable;

		return GlobalResourceManager::GetDescriptorTable();
	}

	template<>
	void RenderContext::SetConstant(const std::string& name, const ResourceHandle& data)
	{
		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		RHI::ShaderRenderGraphConstantsData constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		if (uniform.type.baseType == RHI::ShaderUniformBaseType::Buffer)
		{
			VT_ENSURE(m_currentPassNode->ReadsResource(data));
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
		//else if (uniform.type.baseType == RHI::ShaderUniformBaseType::Sampler) // #TODO_Ivar: Should we validate samplers?
		//{
		//	VT_ENSURE(m_currentPassNode->ReadsResource(data));
		//}
#endif

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &data, sizeof(ResourceHandle));
	}
}
