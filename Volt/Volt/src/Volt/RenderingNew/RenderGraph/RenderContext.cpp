#include "vtpch.h"
#include "RenderContext.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>

#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

namespace Volt
{
	RenderContext::RenderContext(Ref<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	void RenderContext::BeginRendering(const RenderingInfo& renderingInfo)
	{
		m_commandBuffer->SetViewports({ renderingInfo.viewport });
		m_commandBuffer->SetScissors({ renderingInfo.scissor });
		m_commandBuffer->BeginRendering(renderingInfo.renderingInfo);
	}

	void RenderContext::EndRendering()
	{
		m_commandBuffer->EndRendering();
	}

	void RenderContext::Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
	{
		BindDescriptorTableIfRequired();

		m_commandBuffer->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::DrawIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
	{
		m_commandBuffer->DrawIndirectCount(commandsBuffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
	}

	void RenderContext::BindPipeline(Ref<RHI::RenderPipeline> pipeline)
	{
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

	void RenderContext::SetDescriptorExternalTable(Ref<RHI::DescriptorTable> descriptorTable)
	{
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
		m_commandBuffer->PushConstants(data, size, offset);
	}

	void RenderContext::SetBufferView(Ref<RHI::BufferView> view, const uint32_t set, const uint32_t binding, const uint32_t arrayIndex)
	{
		if (!m_currentDescriptorTable)
		{
			return;
		}

		m_currentDescriptorTable->SetBufferView(view, set, binding, arrayIndex);
	}

	void RenderContext::BindDescriptorTableIfRequired()
	{
		if (m_descriptorTableIsBound)
		{
			return;
		}

		m_commandBuffer->BindDescriptorTable(m_currentDescriptorTable);
		m_descriptorTableIsBound = true;
	}

	Ref<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(Ref<RHI::RenderPipeline> renderPipeline)
	{
		void* ptr = renderPipeline.get();

		if (m_descriptorTableCache.contains(ptr))
		{
			return m_descriptorTableCache.at(ptr);
		}

		RHI::DescriptorTableCreateInfo info{};
		info.shader = renderPipeline->GetShader();

		Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		m_descriptorTableCache[ptr] = descriptorTable;

		return descriptorTable;
	}

	Ref<RHI::DescriptorTable> RenderContext::GetOrCreateDescriptorTable(Ref<RHI::ComputePipeline> computePipeline)
	{
		void* ptr = computePipeline.get();

		if (m_descriptorTableCache.contains(ptr))
		{
			return m_descriptorTableCache.at(ptr);
		}

		RHI::DescriptorTableCreateInfo info{};
		info.shader = computePipeline->GetShader();

		Ref<RHI::DescriptorTable> descriptorTable = RHI::DescriptorTable::Create(info);
		m_descriptorTableCache[ptr] = descriptorTable;

		return descriptorTable;
	}
}
