#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt
{
	class CommandBuffer : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(CommandBuffer);
		~CommandBuffer() override = default;

		virtual void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) = 0;
		virtual void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) = 0;

		inline const QueueType GetQueueType() const { return m_queueType; }

		static Ref<CommandBuffer> Create(const uint32_t count, QueueType queueType);

	protected:
		CommandBuffer(QueueType queueType);
	
		QueueType m_queueType = QueueType::Graphics;
	};
}