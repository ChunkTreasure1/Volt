#pragma once

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <vector>

struct VkCommandBuffer_T;
struct VkCommandPool_T;
struct VkFence_T;

struct VkQueryPool_T;
struct VkPipelineLayout_T;

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
		void WaitForLastFence() override;
		void WaitForFences() override;

		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) override;
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) override;
		void DrawIndexedIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DrawIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DrawIndexedIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;
		void DrawIndirectCount(Ref<StorageBuffer> commandsBuffer, const size_t offset, Ref<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) override;
		void DispatchIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset) override;

		void SetViewports(const std::vector<Viewport>& viewports) override;
		void SetScissors(const std::vector<Rect2D>& scissors) override;

		void BindPipeline(Ref<RenderPipeline> pipeline) override;
		void BindPipeline(Ref<ComputePipeline> pipeline) override;
		void BindVertexBuffers(const std::vector<Ref<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding) override;
		void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) override;
		void BindDescriptorTable(Ref<DescriptorTable> descriptorTable) override;

		void BeginRendering(const RenderingInfo& renderingInfo) override;
		void EndRendering() override;

		void PushConstants(const void* data, const uint32_t size, const uint32_t offset) override;

		void ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers) override;

		void BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor) override;
		void EndMarker();

		const uint32_t BeginTimestamp() override;
		void EndTimestamp(uint32_t timestampIndex) override;
		const float GetExecutionTime(uint32_t timestampIndex) const override;

		void CopyImageToBackBuffer(Ref<Image2D> srcImage) override;
		void ClearImage(Ref<Image2D> image, std::array<float, 4> clearColor) override;
		void ClearBuffer(Ref<StorageBuffer> buffer, const uint32_t value) override;

		void CopyBufferRegion(Ref<Allocation> srcAllocation, const size_t srcOffset, Ref<Allocation> dstAllocation, const size_t dstOffset, const size_t size) override;
		void CopyBufferToImage(Ref<Allocation> srcBuffer, Ref<Image2D> dstImage, const uint32_t width, const uint32_t height, const uint32_t mip /* = 0 */) override;
		void CopyImage(Ref<Image2D> srcImage, Ref<Image2D> dstImage, const uint32_t width, const uint32_t height) override;

		const uint32_t GetCurrentIndex() const override;

		VkFence_T* GetCurrentFence() const;

	protected:
		void* GetHandleImpl() const override;

	private:
		friend class VulkanDescriptorTable;
		friend class VulkanDescriptorBufferTable;

		inline static constexpr uint32_t MAX_QUERIES = 64;

		void Invalidate();
		void Release();

		void CreateQueryPools();
		void FetchTimestampResults();

		// #TODO_Ivar: Maybe move somewhere else
		void CheckWaitReturnValue(uint32_t resultValue);

		VkPipelineLayout_T* GetCurrentPipelineLayout();
		const uint32_t GetCurrentCommandBufferIndex() const;

		struct CommandBufferData
		{
			VkCommandBuffer_T* commandBuffer = nullptr;
			VkCommandPool_T* commandPool = nullptr;
			VkFence_T* fence = nullptr;
		};

		std::vector<CommandBufferData> m_commandBuffers;

		uint32_t m_currentCommandBufferIndex = 0;
		uint32_t m_lastCommandBufferIndex = 0;

		bool m_isSwapchainTarget = false;
		bool m_hasTimestampSupport = false;

		uint32_t m_commandBufferCount = 0;

		// Queries
		uint32_t m_timestampQueryCount = 0;
		uint32_t m_nextAvailableTimestampQuery = 0; // The two first are command buffer total
		uint32_t m_lastAvailableTimestampQuery = 0;

		std::vector<VkQueryPool_T*> m_timestampQueryPools;
		std::vector<uint32_t> m_timestampCounts;
		std::vector<std::vector<uint64_t>> m_timestampQueryResults;
		std::vector<std::vector<float>> m_executionTimes;

		// Internal state
		Weak<RenderPipeline> m_currentRenderPipeline;
		Weak<ComputePipeline> m_currentComputePipeline;
	};
}
