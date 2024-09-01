#include "rhipch.h"
#include "RHIModule/Buffers/CommandBufferSet.h"

namespace Volt::RHI
{
	CommandBufferSet::CommandBufferSet(const uint32_t count, QueueType queueType)
		: m_count(count)
	{
		m_commandBuffers.resize(m_count);
		for (uint32_t i = 0; i < m_count; i++)
		{
			m_commandBuffers[i] = CommandBuffer::Create(queueType);
		}
	}

	RefPtr<CommandBuffer> CommandBufferSet::GetCurrentCommandBuffer() const
	{
		return m_commandBuffers.at(m_currentIndex);
	}
	
	RefPtr<CommandBuffer> CommandBufferSet::IncrementAndGetCommandBuffer()
	{
		m_currentIndex = (m_currentIndex + 1) % m_count;
		return m_commandBuffers.at(m_currentIndex);
	}
	
	void CommandBufferSet::Increment()
	{
		m_currentIndex = (m_currentIndex + 1) % m_count;
	}
}
