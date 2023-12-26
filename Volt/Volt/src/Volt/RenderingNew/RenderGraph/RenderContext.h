#pragma once

#include "Volt/Core/Base.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphCommon.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <VoltRHI/Core/RHICommon.h>
#include <VoltRHI/Shader/Shader.h>
	
#include <glm/glm.hpp>

namespace Volt
{
	namespace RHI
	{
		class CommandBuffer;
		class StorageBuffer;

		class RenderPipeline;
		class ComputePipeline;
		class DescriptorTable;

		class BufferView;
		class ImageView;

		class Image2D;
		class BufferViewSet;
	}

	struct RenderingInfo
	{
		RHI::Rect2D scissor{};
		RHI::Viewport viewport{};
		RHI::RenderingInfo renderingInfo{};
	};

	class RenderContext
	{
	public:
		RenderContext(Ref<RHI::CommandBuffer> commandBuffer);
		~RenderContext();

		void BeginRendering(const RenderingInfo& renderingInfo);
		void EndRendering();

		const RenderingInfo CreateRenderingInfo(const uint32_t width, const uint32_t height, const std::vector<Ref<RHI::ImageView>>& attachments);

		void ClearImage(Ref<RHI::Image2D> image, const glm::vec4& clearColor);
		void ClearBuffer(Ref<RHI::StorageBuffer> buffer, uint32_t clearValue);

		void UploadBufferData(Ref<RHI::StorageBuffer> buffer, const void* data, const size_t size);

		void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchMeshTasksIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride);
		void DispatchMeshTasksIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset);
		
		void DrawIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);
		void DrawIndexedIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride);

		void BindPipeline(Ref<RHI::RenderPipeline> pipeline);
		void BindPipeline(Ref<RHI::ComputePipeline> pipeline);

		void BindIndexBuffer(Ref<RHI::StorageBuffer> indexBuffer);

		void Flush();
		Ref<RHI::StorageBuffer> GetReadbackBuffer(Ref<RHI::StorageBuffer> buffer);

		template<typename T>
		void SetConstant(const T& data);

	private:
		friend class RenderGraph;

		void BindDescriptorTableIfRequired();

		void SetPassConstantsBuffer(Weak<RHI::StorageBuffer> constantsBuffer);
		void SetCurrentPassIndex(const uint32_t passIndex);
		void UploadConstantsData();

		Ref<RHI::DescriptorTable> GetOrCreateDescriptorTable(Ref<RHI::RenderPipeline> renderPipeline);
		Ref<RHI::DescriptorTable> GetOrCreateDescriptorTable(Ref<RHI::ComputePipeline> computePipeline);

		// Internal state
		bool m_descriptorTableIsBound = false; // This needs to be checked in every call that uses resources

		Weak<RHI::RenderPipeline> m_currentRenderPipeline;
		Weak<RHI::ComputePipeline> m_currentComputePipeline;
		Ref<RHI::DescriptorTable> m_currentDescriptorTable;

		Weak<RHI::StorageBuffer> m_passConstantsBuffer;
		uint32_t m_currentPassIndex = 0;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		std::unordered_map<void*, Ref<RHI::DescriptorTable>> m_descriptorTableCache;
	
		// #TODO_Ivar: Should be changed
		std::vector<uint8_t> m_passConstantsBufferData;
		uint32_t m_currentPassConstantsOffset = 0;
	};

	template<typename T>
	inline void RenderContext::SetConstant(const T& data)
	{
		// #TODO_Ivar: Add validation

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + m_currentPassConstantsOffset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE - m_currentPassConstantsOffset, &data, sizeof(T));
		m_currentPassConstantsOffset += sizeof(T);
	}
}
