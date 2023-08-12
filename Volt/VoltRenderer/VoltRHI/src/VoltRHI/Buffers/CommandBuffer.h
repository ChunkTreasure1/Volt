#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

#include <span>

namespace Volt::RHI
{
	class RenderPipeline;
	class ImageView;

	class VertexBuffer;
	class IndexBuffer;

	class DescriptorTable;

	class Image2D;
	class StorageBuffer;

	class CommandBuffer : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(CommandBuffer);
		~CommandBuffer() override = default;

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Execute() = 0;
		virtual void ExecuteAndWait() = 0;

		virtual void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) = 0;
		virtual void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance) = 0;
		virtual void DrawIndexedIndirect(Ref<StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride) = 0;

		virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;
		virtual void SetScissors(const std::vector<Rect2D>& scissors) = 0;

		virtual void BindPipeline(Ref<RenderPipeline> pipeline) = 0;
		virtual void BindVertexBuffers(const std::vector<Ref<VertexBuffer>>& vertexBuffers, const uint32_t firstBinding) = 0;
		virtual void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;
		virtual void BindDescriptorTable(Ref<DescriptorTable> descriptorTable) = 0;

		virtual void BeginRendering(const RenderingInfo& renderingInfo) = 0;
		virtual void EndRendering() = 0;

		virtual void PushConstants(const void* data, const uint32_t size, const uint32_t offset) = 0;

		virtual void ResourceBarrier(const std::vector<ResourceBarrierInfo>& resourceBarriers) = 0;

		virtual const uint32_t BeginTimestamp() = 0;
		virtual void EndTimestamp(uint32_t timestampIndex) = 0;
		virtual const float GetExecutionTime(uint32_t timestampIndex) const = 0;

		virtual void CopyImageToBackBuffer(Ref<Image2D> srcImage) = 0;
		virtual void ClearImage(Ref<Image2D> image, std::array<float, 4> clearColor) = 0;

		inline const QueueType GetQueueType() const { return m_queueType; }

		static Ref<CommandBuffer> Create(const uint32_t count, QueueType queueType = QueueType::Graphics, bool swapchainTarget = false);
		static Ref<CommandBuffer> Create();

	protected:
		CommandBuffer(QueueType queueType);
	
		QueueType m_queueType = QueueType::Graphics;
	};
}
