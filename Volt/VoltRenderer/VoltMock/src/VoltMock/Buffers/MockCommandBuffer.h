#pragma once

#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt::RHI
{
	class MockCommandBuffer : public CommandBuffer
	{
	public:
		MockCommandBuffer(const uint32_t count, QueueType queueType);
		~MockCommandBuffer() override = default;

		void Begin() override;
		void End() override;
		void Execute() override;
		void ExecuteAndWait() override;

		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) override;
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) override;

		void SetScissors(const std::vector<Rect2D>& scissors) override;
		void SetViewports(const std::vector<Viewport>& viewports) override;

	protected:
		void* GetHandleImpl() override;
	};
}
