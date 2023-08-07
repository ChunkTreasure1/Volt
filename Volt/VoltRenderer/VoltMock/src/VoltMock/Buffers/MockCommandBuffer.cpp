#include "mkpch.h"
#include "MockCommandBuffer.h"

namespace Volt::RHI
{
	MockCommandBuffer::MockCommandBuffer(const uint32_t count, QueueType queueType)
		: CommandBuffer(queueType)
	{
	}

	void MockCommandBuffer::Begin()
	{
	}

	void MockCommandBuffer::End()
	{
	}

	void MockCommandBuffer::Execute()
	{
	}

	void MockCommandBuffer::ExecuteAndWait()
	{
	}

	void MockCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
	}

	void MockCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
	}

	void MockCommandBuffer::SetScissors(const std::vector<Rect2D>& scissors)
	{
	}

	void MockCommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
	{
	}

	void MockCommandBuffer::BindPipeline(Ref<RenderPipeline> pipeline)
	{
	}

	void MockCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
	}

	void MockCommandBuffer::EndRendering()
	{
	}
	
	void* MockCommandBuffer::GetHandleImpl()
	{
		return nullptr;
	}
}
