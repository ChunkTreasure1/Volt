#pragma once

#include "RenderCore/Config.h"

#include "RenderCore/RenderGraph/RenderGraphPass.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "RenderCore/RenderGraph/RenderContext.h"
#include "RenderCore/RenderGraph/SharedRenderContext.h"
#include "RenderCore/TransientResourceSystem/TransientResourceSystem.h" 

#include "RenderCore/Debug/ShaderRuntimeValidator.h"

#include <RHIModule/Core/ResourceStateTracker.h>

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/Containers/ThreadSafeVector.h>

#include <string_view>
#include <functional>

// TODO:
// * Implement validation system 
// * Implement warning system

namespace Volt
{
	namespace RHI
	{
		class CommandBuffer;
		class Image2D;
		class StorageBuffer;
		class UniformBuffer;
		class MemoryPool;
	}

	class RenderGraphPassResources;
	struct RenderGraphResourceNodeBase;		
	struct RenderGraphPassNodeBase;

	struct RenderGraphImageDesc;
	struct RenderGraphBufferDesc;

	class GPUReadbackBuffer;
	class GPUReadbackImage;

	struct ResourceUsageInfo
	{
		RenderGraphResourceHandle resourceHandle;
		ResourceType type;
		RHI::ResourceState newState;
	};

	class VTRC_API RenderGraph
	{
	public:
		typedef std::function<void(RefPtr<RHI::CommandBuffer> commandBuffer)> MarkerFunction;
		typedef std::function<void(const uint64_t allocatedSize)> TotalAllocatedSizeCallback;

		RenderGraph(RefPtr<RHI::CommandBuffer> commandBuffer);
		~RenderGraph();

		RenderGraph(RenderGraph&& other) noexcept;
		RenderGraph& operator=(RenderGraph&& other) noexcept;

		RenderGraph(const RenderGraph& other) = delete;
		RenderGraph& operator=(const RenderGraph& other) = delete;

		class VTRC_API Builder
		{
		public:
			Builder(RenderGraph& renderGraph, Ref<RenderGraphPassNodeBase> pass);

			RenderGraphImageHandle CreateImage(const RenderGraphImageDesc& textureDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);
			RenderGraphBufferHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);
			RenderGraphUniformBufferHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);

			RenderGraphImageHandle AddExternalImage(RefPtr<RHI::Image> image);
			RenderGraphBufferHandle AddExternalBuffer(RefPtr<RHI::StorageBuffer> buffer);
			RenderGraphUniformBufferHandle AddExternalUniformBuffer(RefPtr<RHI::UniformBuffer> buffer);

			void SetHasSideEffect();
			void SetIsComputePass();

			void ReadResource(RenderGraphResourceHandle handle, RenderGraphResourceState forceState = RenderGraphResourceState::None);
			void WriteResource(RenderGraphResourceHandle handle, RenderGraphResourceState forceState = RenderGraphResourceState::None);
		
		private:
			RenderGraph& m_renderGraph;
			Weak<RenderGraphPassNodeBase> m_pass;
		};

		void Compile();

		// NOTE: After calling Execute the RenderGraph object is no longer valid to use!
		void Execute();
		void ExecuteImmediate();
		void ExecuteImmediateAndWait();

		template<typename T>
		T& AddPass(const std::string& name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, RenderContext&)>&& executeFunc);
		void AddPass(const std::string& name, std::function<void(Builder&)> createFunc, std::function<void(RenderContext&)>&& executeFunc);

		void AddMappedBufferUpload(RenderGraphBufferHandle bufferHandle, const void* data, const size_t size, std::string_view name);
		void AddMappedBufferUpload(RenderGraphUniformBufferHandle bufferHandle, const void* data, const size_t size, std::string_view name);
		void AddStagedBufferUpload(RenderGraphBufferHandle bufferHandle, const void* data, const size_t size, std::string_view name);

		void AddResourceBarrier(RenderGraphResourceHandle resourceHandle, const RenderGraphBarrierInfo& barrierInfo);
		
		Ref<GPUReadbackBuffer> EnqueueBufferReadback(RenderGraphBufferHandle sourceBuffer);
		Ref<GPUReadbackImage> EnqueueImageReadback(RenderGraphImageHandle sourceImage);

		void EnqueueImageExtraction(RenderGraphImageHandle resourceHandle, RefPtr<RHI::Image>& outImage);
		void EnqueueBufferExtraction(RenderGraphBufferHandle resourceHandle, RefPtr<RHI::StorageBuffer>& outBuffer);

		void BeginMarker(const std::string& markerName, const glm::vec4& markerColor = 1.f);
		void EndMarker();

		void SetTotalAllocatedSizeCallback(TotalAllocatedSizeCallback&& callback);

		RenderGraphImageHandle AddExternalImage(RefPtr<RHI::Image> image);
		RenderGraphBufferHandle AddExternalBuffer(RefPtr<RHI::StorageBuffer> buffer);
		RenderGraphUniformBufferHandle AddExternalUniformBuffer(RefPtr<RHI::UniformBuffer> buffer);

		RenderGraphImageHandle CreateImage(const RenderGraphImageDesc& textureDesc);
		RenderGraphBufferHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc);
		RenderGraphUniformBufferHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc);

		ResourceHandle GetImage(const RenderGraphImageHandle resourceHandle, const int32_t mip = -1, const int32_t layer = -1);
		ResourceHandle GetImageArray(const RenderGraphImageHandle resourceHandle, const int32_t mip = -1);
		ResourceHandle GetBuffer(const RenderGraphBufferHandle resourceHandle);
		ResourceHandle GetUniformBuffer(const RenderGraphUniformBufferHandle resourceHandle);

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		ResourceHandle GetRuntimeShaderValidationErrorBuffer();
#endif

	private:
		friend class RenderGraphPassResources;
		friend class Builder;
		friend class RenderGraphExecutionThread;
		friend class RenderContext;
		friend class RenderContext3;

		struct Image2DExtractionInfo
		{
			RenderGraphImageHandle resourceHandle;
			RefPtr<RHI::Image>* outImagePtr = nullptr;
		};

		struct BufferExtractionInfo
		{
			RenderGraphBufferHandle resourceHandle;
			RefPtr<RHI::StorageBuffer>* outBufferPtr = nullptr;
		};

		struct CompiledRenderGraphPass
		{
			struct BarrierInfo
			{
				RHI::ResourceBarrierInfo barrier;
				RenderGraphResourceHandle resourceHandle = RenderGraphNullHandle{};
			};

			class PassBarriers
			{
			public:
				VT_NODISCARD VT_INLINE std::span<const BarrierInfo> GetBarriers() const { return m_barriers; }
				VT_NODISCARD VT_INLINE size_t GetBarrierCount() const { return m_barriers.size(); }
				VT_NODISCARD VT_INLINE bool Empty() const { return m_barriers.empty(); }

				VT_NODISCARD VT_INLINE RHI::ResourceBarrierInfo& AddBarrier(RHI::BarrierType type, RenderGraphResourceHandle resourceHandle = RenderGraphNullHandle{})
				{
					auto& barrierInfo = m_barriers.emplace_back();
					barrierInfo.barrier.type = type;
					barrierInfo.resourceHandle = resourceHandle;
					return barrierInfo.barrier;
				}

				VT_NODISCARD VT_INLINE RHI::ResourceBarrierInfo& GetBarrier(size_t index)
				{
					return m_barriers.at(index).barrier;
				}

			private:
				Vector<BarrierInfo> m_barriers;
			};

			std::string_view name;
			Vector<RenderGraphResourceHandle> surrenderableResources;
			PassBarriers prePassBarriers;
			PassBarriers postPassBarriers;

			// We only want maximum ONE global barrier per pass. As a single global barrier
			// can represent multiple.
			inline RHI::GlobalBarrier& GetGlobalBarrier()
			{
				if (m_globalBarrierIndex == -1)
				{
					m_globalBarrierIndex = static_cast<int32_t>(prePassBarriers.GetBarrierCount());
					auto& barrier = prePassBarriers.AddBarrier(RHI::BarrierType::Global);
					return barrier.globalBarrier();
				}

				return prePassBarriers.GetBarrier(static_cast<size_t>(m_globalBarrierIndex)).globalBarrier();
			}

			inline RHI::GlobalBarrier& GetPostPassGlobalBarrier()
			{
				if (m_postPassGlobalBarrierIndex == -1)
				{
					m_postPassGlobalBarrierIndex = static_cast<int32_t>(postPassBarriers.GetBarrierCount());
					auto& barrier = postPassBarriers.AddBarrier(RHI::BarrierType::Global);
					return barrier.globalBarrier();
				}

				return postPassBarriers.GetBarrier(static_cast<size_t>(m_postPassGlobalBarrierIndex)).globalBarrier();
			}

		private:
			int32_t m_globalBarrierIndex = -1;
			int32_t m_postPassGlobalBarrierIndex = -1;
		};

		class StandaloneBarriers
		{
		public:
			VT_NODISCARD VT_INLINE ResourceUsageInfo& AddBarrier(uint32_t passIndex) { return m_passBarriers[passIndex].emplace_back(); }
			VT_NODISCARD VT_INLINE std::span<const ResourceUsageInfo> GetPassBarriers(uint32_t passIndex) const { return m_passBarriers.at(passIndex); }
			VT_NODISCARD VT_INLINE bool HasPassBarriers(uint32_t passIndex) const { return m_passBarriers.contains(passIndex) && !m_passBarriers.at(passIndex).empty(); }

		private:
			vt::map<uint32_t, Vector<ResourceUsageInfo>> m_passBarriers;
		};

		void ExecuteInternal(bool waitForCompletedExecution, bool waitForSync);

		void DestroyResources();
		void AllocateConstantsBuffer();
		void ExtractResources();

		void InitializeRuntimeShaderValidator();
		void AddRuntimeShaderValidationBuffers(Builder& builder);

		void PrintPassBarriers(const Vector<RHI::ResourceBarrierInfo>& barriers);

		WeakPtr<RHI::ImageView> GetImageView(const RenderGraphImageHandle resourceHandle);
		WeakPtr<RHI::Image> GetImageRaw(const RenderGraphImageHandle resourceHandle);
		WeakPtr<RHI::StorageBuffer> GetBufferRaw(const RenderGraphBufferHandle resourceHandle);
		WeakPtr<RHI::StorageBuffer> GetUniformBufferRaw(const RenderGraphUniformBufferHandle resourceHandle);
		WeakPtr<RHI::RHIResource> GetResourceRaw(const RenderGraphResourceHandle resourceHandle);

		RefPtr<RHI::Image> GetImageRawRef(const RenderGraphImageHandle resourceHandle);
		RefPtr<RHI::StorageBuffer> GetBufferRawRef(const RenderGraphBufferHandle resourceHandle);
		RefPtr<RHI::StorageBuffer> GetUniformBufferRawRef(const RenderGraphUniformBufferHandle resourceHandle);

		RenderGraphResourceHandle TryGetRegisteredExternalResource(WeakPtr<RHI::RHIResource> resource);
		void RegisterExternalResource(WeakPtr<RHI::RHIResource> resource, RenderGraphResourceHandle handle);

		void InsertBarriersIntoCommandBuffer(const CompiledRenderGraphPass::PassBarriers& passBarriers, const RefPtr<RHI::CommandBuffer>& commandBuffer);
		void InsertStandaloneMarkersIntoCommandBuffer(const uint32_t passIndex, const RefPtr<RHI::CommandBuffer> commandBuffer);

		Vector<Image2DExtractionInfo> m_imageExtractions;
		Vector<BufferExtractionInfo> m_bufferExtractions;

		Vector<Vector<MarkerFunction>> m_standaloneMarkers; // Pass -> Markers
		Vector<Ref<RenderGraphPassNodeBase>> m_passNodes;
		Vector<Ref<RenderGraphResourceNodeBase>> m_resourceNodes;

		StandaloneBarriers m_standaloneBarriers;

		Vector<CompiledRenderGraphPass> m_compiledPasses;
		
		vt::map<WeakPtr<RHI::RHIResource>, RenderGraphResourceHandle> m_registeredExternalResources;

		Vector<uint8_t*> m_temporaryAllocations;

		struct RegisteredImageView
		{
			ResourceHandle handle;
			RHI::ImageViewType viewType;
		};

		ThreadSafeVector<ResourceHandle> m_registeredResources;

		uint32_t m_passIndex = 0;
		uint32_t m_resourceIndex = 0;

		RefPtr<RHI::CommandBuffer> m_commandBuffer;
		RefPtr<RHI::StorageBuffer> m_perPassConstantsBuffer;
		WeakPtr<RHI::UniformBuffer> m_renderGraphConstantsBuffer;

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		ShaderRuntimeValidator m_runtimeShaderValidator;
#endif

		TransientResourceSystem m_transientResourceSystem;

		SharedRenderContext m_sharedRenderContext;

		bool m_currentlyInBuilder = false;
		bool m_hasBeenCompiled = false;

		TotalAllocatedSizeCallback m_totalAllocatedSizeCallback;
	};

	template<typename T>
	inline T& RenderGraph::AddPass(const std::string& name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, RenderContext&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		
		Ref<RenderGraphPassNode<T>> newNode = CreateRef<RenderGraphPassNode<T>>();
		newNode->name = name;
		newNode->executeFunction = executeFunc;
		newNode->index = m_passIndex++;

		m_passNodes.push_back(newNode);
		m_standaloneMarkers.emplace_back();

		m_currentlyInBuilder = true;
		Builder builder{ *this, newNode };
		createFunc(builder, newNode->data);

		AddRuntimeShaderValidationBuffers(builder);
		m_currentlyInBuilder = false;

		return newNode->data;
	}
}
