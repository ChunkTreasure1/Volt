#pragma once

#include "Volt/Core/Base.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <VoltRHI/Core/RHICommon.h>

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

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset);
		void DrawIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);

		void BindPipeline(Ref<RHI::RenderPipeline> pipeline, Ref<RHI::DescriptorTable> externalDescriptorTable = nullptr);
		void BindPipeline(Ref<RHI::ComputePipeline> pipeline, Ref<RHI::DescriptorTable> externalDescriptorTable = nullptr);

		void SetExternalDescriptorTable(Ref<RHI::DescriptorTable> descriptorTable);

		void PushConstantsRaw(const void* data, const uint32_t size, const uint32_t offset = 0);

		template<typename T>
		void PushConstants(const T& data, const uint32_t offset = 0);

		void SetBufferView(Ref<RHI::BufferView> view, const uint32_t set, const uint32_t binding, const uint32_t arrayIndex = 0);
		void SetBufferView(std::string_view name, Ref<RHI::BufferView> view, const uint32_t arrayIndex = 0);
		void SetBufferViews(const std::vector<Ref<RHI::BufferView>>& views, const uint32_t set, const uint32_t binding, const uint32_t arrayStartOffset = 0);
		void SetBufferViewSet(Ref<RHI::BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0);

		void SetImageView(Ref<RHI::ImageView> view, const uint32_t set, const uint32_t binding, const uint32_t arrayIndex = 0);
		void SetImageView(std::string_view name, Ref<RHI::ImageView> view, const uint32_t arrayIndex = 0);
		void SetImageViews(const std::vector<Ref<RHI::ImageView>>& views, const uint32_t set, const uint32_t binding, const uint32_t arrayStartOffset = 0);

	private:
		void BindDescriptorTableIfRequired();

		Ref<RHI::DescriptorTable> GetOrCreateDescriptorTable(Ref<RHI::RenderPipeline> renderPipeline);
		Ref<RHI::DescriptorTable> GetOrCreateDescriptorTable(Ref<RHI::ComputePipeline> computePipeline);

		// Internal state
		bool m_descriptorTableIsBound = false; // This needs to be checked in every call that uses resources

		Weak<RHI::RenderPipeline> m_currentRenderPipeline;
		Weak<RHI::ComputePipeline> m_currentComputePipeline;
		Ref<RHI::DescriptorTable> m_currentDescriptorTable;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		std::unordered_map<void*, Ref<RHI::DescriptorTable>> m_descriptorTableCache;
	};

	template<typename T>
	inline void RenderContext::PushConstants(const T& data, const uint32_t offset)
	{
		PushConstantsRaw(&data, sizeof(T), offset);
	}
}