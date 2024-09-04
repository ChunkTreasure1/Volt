#pragma once

#include "VulkanRHIModule/Core.h"
#include <RHIModule/Buffers/CommandBuffer.h>

struct VkCommandBuffer_T;
struct VkCommandPool_T;
struct VkFence_T;

struct VkQueryPool_T;
struct VkPipelineLayout_T;

namespace Volt::RHI
{
	class Semaphore;

	class VulkanCommandBuffer final : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(QueueType queueType);
		~VulkanCommandBuffer() override;

		void Begin() override;
		void End() override;
		void Execute() override;

		void Flush(RefPtr<Fence> fence) override;
		void ExecuteAndWait() override;
		void WaitForFence() override;

		void SetEvent(WeakPtr<Event> event) override;

		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) override;
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) override;
		void DrawIndexedIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DrawIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DrawIndexedIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;
		void DrawIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) override;
		void DispatchIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset) override;

		void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) override;
		void DispatchMeshTasksIndirect(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) override;
		void DispatchMeshTasksIndirectCount(WeakPtr<StorageBuffer> commandsBuffer, const size_t offset, WeakPtr<StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) override;

		void SetViewports(const StackVector<Viewport, MAX_VIEWPORT_COUNT>& viewports) override;
		void SetScissors(const StackVector<Rect2D, MAX_VIEWPORT_COUNT>& scissors) override;

		void BindPipeline(WeakPtr<RenderPipeline> pipeline) override;
		void BindPipeline(WeakPtr<ComputePipeline> pipeline) override;
		void BindVertexBuffers(const StackVector<WeakPtr<VertexBuffer>, MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding) override;
		void BindVertexBuffers(const StackVector<WeakPtr<StorageBuffer>, MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding) override;
		void BindIndexBuffer(WeakPtr<IndexBuffer> indexBuffer) override;
		void BindIndexBuffer(WeakPtr<StorageBuffer> indexBuffer) override;

		void BindDescriptorTable(WeakPtr<DescriptorTable> descriptorTable) override;
		void BindDescriptorTable(WeakPtr<BindlessDescriptorTable> descriptorTable) override;

		void BeginRendering(const RenderingInfo& renderingInfo) override;
		void EndRendering() override;

		void PushConstants(const void* data, const uint32_t size, const uint32_t offset) override;

		void ResourceBarrier(const Vector<ResourceBarrierInfo>& resourceBarriers) override;

		void BeginMarker(std::string_view markerLabel, const std::array<float, 4>& markerColor) override;
		void EndMarker() override;

		const uint32_t BeginTimestamp() override;
		void EndTimestamp(uint32_t timestampIndex) override;
		const float GetExecutionTime(uint32_t timestampIndex) const override;

		void ClearImage(WeakPtr<Image> image, std::array<float, 4> clearColor) override;
		void ClearBuffer(WeakPtr<StorageBuffer> buffer, const uint32_t value) override;

		void UpdateBuffer(WeakPtr<StorageBuffer> dstBuffer, const size_t dstOffset, const size_t dataSize, const void* data) override;
		void CopyBufferRegion(WeakPtr<Allocation> srcAllocation, const size_t srcOffset, WeakPtr<Allocation> dstAllocation, const size_t dstOffset, const size_t size) override;
		void CopyBufferToImage(WeakPtr<Allocation> srcBuffer, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip /* = 0 */) override;
		void CopyImageToBuffer(WeakPtr<Image> srcImage, WeakPtr<Allocation> dstBuffer, const size_t dstOffset, const uint32_t width, const uint32_t height, const uint32_t depth, const uint32_t mip) override;
		void CopyImage(WeakPtr<Image> srcImage, WeakPtr<Image> dstImage, const uint32_t width, const uint32_t height, const uint32_t depth) override;

		void UploadTextureData(WeakPtr<Image> dstImage, const ImageCopyData& copyData) override;

		const QueueType GetQueueType() const override;
		const WeakPtr<Fence> GetFence() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		friend class VulkanDescriptorTable;
		friend class VulkanDescriptorBufferTable;
		friend class VulkanBindlessDescriptorTable;

		inline static constexpr uint32_t MAX_QUERIES = 64;

		void Invalidate();
		void Release(RefPtr<Fence> waitFence);

		void CreateQueryPools();
		void FetchTimestampResults();

		VkPipelineLayout_T* GetCurrentPipelineLayout();

		struct CommandBufferData
		{
			VkCommandBuffer_T* commandBuffer = nullptr;
			VkCommandPool_T* commandPool = nullptr;
		};

		RefPtr<Fence> m_fence;
		CommandBufferData m_commandBufferData;

		bool m_hasTimestampSupport = false;

		QueueType m_queueType;

		// Queries
		uint32_t m_timestampQueryCount = 0;
		uint32_t m_nextAvailableTimestampQuery = 0; // The two first are command buffer total
		uint32_t m_lastAvailableTimestampQuery = 0;

		VkQueryPool_T* m_timestampQueryPool;
		uint32_t m_timestampCount;
		Vector<uint64_t> m_timestampQueryResults;
		Vector<float> m_executionTimes;

		// Internal state
		WeakPtr<RenderPipeline> m_currentRenderPipeline;
		WeakPtr<ComputePipeline> m_currentComputePipeline;
	};
}
