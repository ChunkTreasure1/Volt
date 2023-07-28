#pragma once

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <vector>

struct VkCommandBuffer_T;
struct VkCommandPool_T;
struct VkFence_T;

namespace Volt::RHI
{
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(const uint32_t count, QueueType queueType, bool swapchainTarget);
		~VulkanCommandBuffer() override;

		void Begin() override;
		void End() override;
		void Execute() override;
		void ExecuteAndWait() override;

		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) override;
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) override;

		void SetViewports(const std::vector<Viewport>& viewports) override;
		void SetScissors(const std::vector<Rect2D>& scissors) override;

		VkFence_T* GetCurrentFence() const;

	protected:
		void* GetHandleImpl() override;

	private:
		void Invalidate();
		void Release();

		const uint32_t GetCurrentCommandBufferIndex() const;

		struct CommandBufferData
		{
			VkCommandBuffer_T* commandBuffer = nullptr;
			VkCommandPool_T* commandPool = nullptr;
			VkFence_T* fence = nullptr;
		};

		std::vector<CommandBufferData> m_commandBuffers;

		uint32_t m_currentCommandBufferIndex = 0;
		bool m_isSwapchainTarget = false;

		uint32_t m_commandBufferCount;
	};
}
