#include "vtpch.h"
#include "RenderContext.h"

#include "Volt/Core/Profiling.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>

#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

#include <VoltRHI/Graphics/GraphicsContext.h>

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

	void RenderContext::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();

		m_commandBuffer->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DrawIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		VT_PROFILE_FUNCTION();

		BindDescriptorTableIfRequired();

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

	void RenderContext::PushConstants(const void* data, const uint32_t size, const uint32_t offset)
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

		Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		m_descriptorTableCache[ptr] = descriptorTable;

		return descriptorTable;
	}
}
