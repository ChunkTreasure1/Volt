#pragma once

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <vector>

struct VkCommandBuffer_T;
struct VkCommandPool_T;
struct VkFence_T;

struct VkQueryPool_T;

namespace Volt::RHI
{
	class VulkanCommandBuffer final : public CommandBuffer
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

		void BindPipeline(Ref<RenderPipeline> pipeline) override;
		void BindVertexBuffers(const std::vector<Ref<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding) override;
		void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) override;
		void BindDescriptorTable(Ref<DescriptorTable> descriptorTable) override;

		void BeginRendering(const RenderingInfo& renderingInfo) override;
		void EndRendering() override;

		void PushConstants(const void* data, const uint32_t size, const uint32_t offset) override;

		void ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers) override;

		const uint32_t BeginTimestamp() override;
		void EndTimestamp(uint32_t timestampIndex) override;
		const float GetExecutionTime(uint32_t timestampIndex) const override;

		VkFence_T* GetCurrentFence() const;

	protected:
		void* GetHandleImpl() override;

	private:
		inline static constexpr uint32_t MAX_QUERIES = 64;

		void Invalidate();
		void Release();

		void CreateQueryPools();
		void FetchTimestampResults();

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
		bool m_hasTimestampSupport = false;

		uint32_t m_commandBufferCount = 0;

		// Queries
		uint32_t m_timestampQueryCount = 0;
		uint32_t m_nextAvailableTimestampQuery = 2; // The two first are command buffer total
		uint32_t m_lastAvailableTimestampQuery = 0;

		std::vector<VkQueryPool_T*> m_timestampQueryPools;
		std::vector<uint32_t> m_timestampCounts;
		std::vector<std::vector<uint64_t>> m_timestampQueryResults;
		std::vector<std::vector<float>> m_executionTimes;

		// Internal state
		Ref<RenderPipeline> m_currentRenderPipeline;
	};
}
