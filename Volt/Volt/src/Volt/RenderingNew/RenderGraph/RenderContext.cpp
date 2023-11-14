#include "RenderContext.h"
#include "vtpch.h"
#include "RenderContext.h"

#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

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

	void RenderContext::SetPassConstantsBuffer(Weak<RHI::StorageBuffer> constantsBuffer)
	{
		m_passConstantsBuffer = constantsBuffer;
		m_passConstantsBufferData.resize(constantsBuffer->GetByteSize());
		memset(m_passConstantsBufferData.data(), 0, m_passConstantsBufferData.size());
	}

	void RenderContext::SetCurrentPassIndex(const uint32_t passIndex)
	{
		m_currentPassIndex = passIndex;
		m_currentPassConstantsOffset = 0;
	}

	void RenderContext::UploadConstantsData()
	{
		uint8_t* mappedPtr = m_passConstantsBuffer->Map<uint8_t>();
		memcpy_s(mappedPtr, m_passConstantsBuffer->GetByteSize(), m_passConstantsBufferData.data(), m_passConstantsBufferData.size());
		m_passConstantsBuffer->Unmap();
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
			uint32_t constatsBufferIndex;
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
}
