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
			m_passConstantsBuffer->Unmap();
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
			m_mappedPassConstantsPointer = m_passConstantsBuffer->Map<uint8_t>();
			m_isPassConstantsMapped = true;
		}

		const uint32_t offset = RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE * passIndex;
		return &m_mappedPassConstantsPointer[offset];
	}

	ResourceHandle SharedRenderContext::GetPassConstantsBufferResourceHandle() const
	{
		return BindlessResourcesManager::Get().GetBufferHandle(m_passConstantsBuffer);
	}

	void SharedRenderContext::SetPerPassConstantsBuffer(WeakPtr<RHI::StorageBuffer> constantsBuffer)
	{
		m_passConstantsBuffer = constantsBuffer;
	}

	void SharedRenderContext::SetRenderGraphConstantsBuffer(WeakPtr<RHI::UniformBuffer> constantsBuffer)
	{
		m_renderGraphConstantsBuffer = constantsBuffer;
	}
}
