#include "RenderContext.h"
#include "vtpch.h"
#include "RenderContext.h"

#include "Volt/Core/Profiling.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
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

		std::vector<RHI::AttachmentInfo> colorAttachments;
		RHI::AttachmentInfo depthAttachment{};

		for (const auto& view : attachments)
		{
			if ((view->GetImageAspect() & RHI::ImageAspect::Color) != RHI::ImageAspect::None)
			{
				auto& attachment = colorAttachments.emplace_back();
				attachment.clearMode = RHI::ClearMode::Clear;
				attachment.clearColor = { 0.f, 0.f, 0.f, 0.f };
				attachment.view = view;
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

	void RenderContext::BindPipeline(Ref<RHI::RenderPipeline> pipeline, Ref<RHI::DescriptorTable> externalDescriptorTable)
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

		if (externalDescriptorTable)
		{
			SetExternalDescriptorTable(externalDescriptorTable);
		}
		else
		{
			m_currentDescriptorTable = GetOrCreateDescriptorTable(pipeline);
		}
	}

	void RenderContext::BindPipeline(Ref<RHI::ComputePipeline> pipeline, Ref<RHI::DescriptorTable> externalDescriptorTable)
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

		if (externalDescriptorTable)
		{
			SetExternalDescriptorTable(externalDescriptorTable);
		}
		else
		{
			m_currentDescriptorTable = GetOrCreateDescriptorTable(pipeline);
		}
	}

	void RenderContext::SetExternalDescriptorTable(Ref<RHI::DescriptorTable> descriptorTable)
	{
		VT_PROFILE_FUNCTION();

		m_descriptorTableIsBound = false;
		m_currentDescriptorTable = descriptorTable;

		if (m_currentRenderPipeline)
		{
			m_descriptorTableCache[m_currentRenderPipeline.Get()] = descriptorTable;
		}
		else
		{
			m_descriptorTableCache[m_currentComputePipeline.Get()] = descriptorTable;
		}
	}

	void RenderContext::PushConstantsRaw(const void* data, const uint32_t size, const uint32_t offset)
	{
		VT_PROFILE_FUNCTION();
		m_commandBuffer->PushConstants(data, size, offset);
	}

	void RenderContext::SetBufferView(Ref<RHI::BufferView> view, const uint32_t set, const uint32_t binding, const uint32_t arrayIndex)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetBufferView(view, set, binding, arrayIndex);
	}

	void RenderContext::SetBufferView(std::string_view name, Ref<RHI::BufferView> view, const uint32_t arrayIndex)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetBufferView(name, view, arrayIndex);
	}

	void RenderContext::SetBufferViews(const std::vector<Ref<RHI::BufferView>>& views, const uint32_t set, const uint32_t binding, const uint32_t arrayStartOffset)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetBufferViews(views, set, binding, arrayStartOffset);
	}

	void RenderContext::SetBufferViewSet(Ref<RHI::BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetBufferViewSet(bufferViewSet, set, binding, arrayIndex);
	}

	void RenderContext::SetImageView(Ref<RHI::ImageView> view, const uint32_t set, const uint32_t binding, const uint32_t arrayIndex)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetImageView(view, set, binding, arrayIndex);
	}

	void RenderContext::SetImageView(std::string_view name, Ref<RHI::ImageView> view, const uint32_t arrayIndex)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetImageView(name, view, arrayIndex);
	}

	void RenderContext::SetImageViews(const std::vector<Ref<RHI::ImageView>>& views, const uint32_t set, const uint32_t binding, const uint32_t arrayStartOffset)
	{
		VT_PROFILE_FUNCTION();

		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetImageViews(views, set, binding, arrayStartOffset);
	}

	void RenderContext::BindDescriptorTableIfRequired()
	{
		VT_PROFILE_FUNCTION();

		if (m_descriptorTableIsBound)
		{
			return;
		}

		m_commandBuffer->BindDescriptorTable(m_currentDescriptorTable);
		m_descriptorTableIsBound = true;
	}

	Ref<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(Ref<RHI::RenderPipeline> renderPipeline)
	{
		VT_PROFILE_FUNCTION();

		auto shader = renderPipeline->GetShader();
		void* ptr = shader.get();

		if (m_descriptorTableCache.contains(ptr))
		{
			return m_descriptorTableCache.at(ptr);
		}

		RHI::DescriptorTableCreateInfo info{};
		info.shader = shader;
		info.count = 1;

		Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		m_descriptorTableCache[ptr] = descriptorTable;

		return descriptorTable;
	}

	Ref<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(Ref<RHI::ComputePipeline> computePipeline)
	{
		VT_PROFILE_FUNCTION();

		auto shader = computePipeline->GetShader();
		void* ptr = shader.get();

		if (m_descriptorTableCache.contains(ptr))
		{
			return m_descriptorTableCache.at(ptr);
		}

		RHI::DescriptorTableCreateInfo info{};
		info.shader = shader;
		info.count = 1;

		Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		m_descriptorTableCache[ptr] = descriptorTable;

		return descriptorTable;
	}
}
