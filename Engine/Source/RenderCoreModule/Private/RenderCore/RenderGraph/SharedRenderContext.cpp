#include "rcpch.h"

#include "RenderCore/RenderGraph/SharedRenderContext.h"
#include "RenderCore/RenderGraph/RenderGraphCommon.h"
#include "RenderCore/Resources/BindlessResourcesManager.h"

#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Buffers/StorageBuffer.h>

namespace Volt
{
	SharedRenderContext::~SharedRenderContext()
	{
		VT_ENSURE(!m_isRenderGraphConstantsMapped);
		VT_ENSURE(!m_isPassConstantsMapped);
	}

	SharedRenderContext::SharedRenderContext(SharedRenderContext&& other) noexcept
		: m_isRenderGraphConstantsMapped(other.m_isRenderGraphConstantsMapped),
		m_mappedRenderGraphConstantsPointer(other.m_mappedRenderGraphConstantsPointer),
		m_isPassConstantsMapped(other.m_isPassConstantsMapped),
		m_mappedPassConstantsPointer(other.m_mappedPassConstantsPointer),
		m_passConstantsBuffer(std::move(other.m_passConstantsBuffer)),
		m_renderGraphConstantsBuffer(std::move(other.m_renderGraphConstantsBuffer))
	{
	}

	SharedRenderContext& SharedRenderContext::operator=(SharedRenderContext&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		m_isRenderGraphConstantsMapped = other.m_isRenderGraphConstantsMapped;
		m_mappedRenderGraphConstantsPointer = other.m_mappedRenderGraphConstantsPointer;
		m_isPassConstantsMapped = other.m_isPassConstantsMapped;
		m_mappedPassConstantsPointer = other.m_mappedPassConstantsPointer;
		m_passConstantsBuffer = std::move(other.m_passConstantsBuffer);
		m_renderGraphConstantsBuffer = std::move(other.m_renderGraphConstantsBuffer);
	
		return *this;
	}

	void SharedRenderContext::BeginContext()
	{
	}

	void SharedRenderContext::EndContext()
	{
		if (m_isRenderGraphConstantsMapped)
		{
			m_renderGraphConstantsBuffer->Unmap();
			m_mappedRenderGraphConstantsPointer = nullptr;
			m_isRenderGraphConstantsMapped = false;
		}

		if (m_isPassConstantsMapped)
		{
			m_passConstantsBuffer->GetResource()->Unmap();
			m_mappedPassConstantsPointer = nullptr;
			m_isPassConstantsMapped = false;
		}
	}

	uint8_t* SharedRenderContext::GetRenderGraphConstantsPointer(uint32_t passIndex)
	{
		if (!m_isRenderGraphConstantsMapped)
		{
			m_mappedRenderGraphConstantsPointer = m_renderGraphConstantsBuffer->Map<uint8_t>();
			m_isRenderGraphConstantsMapped = true;
		}

		const uint32_t offset = m_renderGraphConstantsBuffer->GetSize() * passIndex;
		return &m_mappedRenderGraphConstantsPointer[offset];
	}

	uint8_t* SharedRenderContext::GetPassConstantsPointer(uint32_t passIndex)
	{
		if (!m_isPassConstantsMapped)
		{
			m_mappedPassConstantsPointer = m_passConstantsBuffer->GetResource()->Map<uint8_t>();
			m_isPassConstantsMapped = true;
		}

		const uint32_t offset = RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE * passIndex;
		return &m_mappedPassConstantsPointer[offset];
	}

	ResourceHandle SharedRenderContext::GetPassConstantsBufferResourceHandle() const
	{
		return m_passConstantsBuffer->GetResourceHandle();
	}

	void SharedRenderContext::SetPerPassConstantsBuffer(RefPtr<RHI::StorageBuffer> constantsBuffer)
	{
		m_passConstantsBuffer = BindlessResource<RHI::StorageBuffer>::CreateScopeFromResource(constantsBuffer);
	}

	void SharedRenderContext::SetRenderGraphConstantsBuffer(WeakPtr<RHI::UniformBuffer> constantsBuffer)
	{
		m_renderGraphConstantsBuffer = constantsBuffer;
	}
}
