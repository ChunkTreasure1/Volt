#include "mkpch.h"
#include "MockCommandBuffer.h"

namespace Volt
{
	MockCommandBuffer::MockCommandBuffer(const uint32_t count, QueueType queueType)
		: CommandBuffer(queueType)
	{
	}

	void MockCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
	}

	void MockCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
	}
	
	void* MockCommandBuffer::GetHandleImpl()
	{
		return nullptr;
	}
}
