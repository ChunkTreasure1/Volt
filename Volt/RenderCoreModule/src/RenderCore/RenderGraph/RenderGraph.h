#pragma once

#include "RenderCore/Config.h"

#include "RenderCore/RenderGraph/RenderGraphPass.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "RenderCore/RenderGraph/RenderContext.h"
#include "RenderCore/TransientResourceSystem/TransientResourceSystem.h" 

#include "RenderCore/Debug/ShaderRuntimeValidator.h"

#include <CoreUtilities/Containers/Map.h>

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

	struct ResourceUsageInfo
	{
		int32_t passIndex = -1;
		RenderGraphResourceHandle resourceHandle;
		RHI::ResourceBarrierInfo accessInfo;

		bool isPassSpecificUsage = true;
		bool hasInitialState = true;
	};

	class VTRC_API RenderGraph
	{
	public:
		typedef std::function<void(RefPtr<RHI::CommandBuffer> commandBuffer)> MarkerFunction;
		typedef std::function<void(const uint64_t allocatedSize)> TotalAllocatedSizeCallback;

		RenderGraph(RefPtr<RHI::CommandBuffer> commandBuffer);
		~RenderGraph();

		class VTRC_API Builder
		{
		public:
			Builder(RenderGraph& renderGraph, Ref<RenderGraphPassNodeBase> pass);

			RenderGraphImage2DHandle CreateImage2D(const RenderGraphImageDesc& textureDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);
			RenderGraphImage3DHandle CreateImage3D(const RenderGraphImageDesc& textureDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);
			RenderGraphBufferHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);
			RenderGraphUniformBufferHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc, RenderGraphResourceState forceState = RenderGraphResourceState::None);

			RenderGraphImage2DHandle AddExternalImage2D(RefPtr<RHI::Image2D> image);
			RenderGraphImage3DHandle AddExternalImage3D(RefPtr<RHI::Image3D> image);
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
		
		void EnqueueBufferReadback(RenderGraphBufferHandle sourceBuffer, RefPtr<RHI::StorageBuffer> dstBuffer);
		void EnqueueImage2DExtraction(RenderGraphImage2DHandle resourceHandle, RefPtr<RHI::Image2D>& outImage);
		void EnqueueBufferExtraction(RenderGraphBufferHandle resourceHandle, RefPtr<RHI::StorageBuffer>& outBuffer);

		void BeginMarker(const std::string& markerName, const glm::vec4& markerColor = 1.f);
		void EndMarker();

		void SetTotalAllocatedSizeCallback(TotalAllocatedSizeCallback&& callback);

		RenderGraphImage2DHandle AddExternalImage2D(RefPtr<RHI::Image2D> image);
		RenderGraphImage3DHandle AddExternalImage3D(RefPtr<RHI::Image3D> image);
		RenderGraphBufferHandle AddExternalBuffer(RefPtr<RHI::StorageBuffer> buffer);
		RenderGraphUniformBufferHandle AddExternalUniformBuffer(RefPtr<RHI::UniformBuffer> buffer);

		RenderGraphImage2DHandle CreateImage2D(const RenderGraphImageDesc& textureDesc);
		RenderGraphImage3DHandle CreateImage3D(const RenderGraphImageDesc& textureDesc);
		RenderGraphBufferHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc);
		RenderGraphUniformBufferHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc);

		ResourceHandle GetImage2D(const RenderGraphImage2DHandle resourceHandle, const int32_t mip = -1, const int32_t layer = -1);
		ResourceHandle GetImage2DArray(const RenderGraphImage2DHandle resourceHandle, const int32_t mip = -1);
		ResourceHandle GetImage3D(const RenderGraphImage3DHandle resourceHandle, const int32_t mip = -1, const int32_t layer = -1);
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

		inline static constexpr RenderGraphResourceHandle INVALID_RESOURCE_HANDLE = std::numeric_limits<RenderGraphResourceHandle>::max();

		void ExecuteInternal(bool waitForCompletion);
		void InsertStandaloneMarkers(const uint32_t passIndex);
		void DestroyResources();
		void AllocateConstantsBuffer();
		void ExtractResources();

		void InitializeRuntimeShaderValidator();
		void AddRuntimeShaderValidationBuffers(Builder& builder);

		struct Image2DExtractionInfo
		{
			RenderGraphImage2DHandle resourceHandle;
			RefPtr<RHI::Image2D>* outImagePtr = nullptr;
		};

		struct BufferExtractionInfo
		{
			RenderGraphBufferHandle resourceHandle;
			RefPtr<RHI::StorageBuffer>* outBufferPtr = nullptr;
		};

		WeakPtr<RHI::ImageView> GetImage2DView(const RenderGraphImage2DHandle resourceHandle);
		WeakPtr<RHI::Image2D> GetImage2DRaw(const RenderGraphImage2DHandle resourceHandle);
		WeakPtr<RHI::Image3D> GetImage3DRaw(const RenderGraphImage3DHandle resourceHandle);
		WeakPtr<RHI::StorageBuffer> GetBufferRaw(const RenderGraphBufferHandle resourceHandle);
		WeakPtr<RHI::StorageBuffer> GetUniformBufferRaw(const RenderGraphUniformBufferHandle resourceHandle);
		WeakPtr<RHI::RHIResource> GetResourceRaw(const RenderGraphResourceHandle resourceHandle);

		RefPtr<RHI::Image2D> GetImage2DRawRef(const RenderGraphImage2DHandle resourceHandle);
		RefPtr<RHI::Image3D> GetImage3DRawRef(const RenderGraphImage3DHandle resourceHandle);
		RefPtr<RHI::StorageBuffer> GetBufferRawRef(const RenderGraphBufferHandle resourceHandle);
		RefPtr<RHI::StorageBuffer> GetUniformBufferRawRef(const RenderGraphUniformBufferHandle resourceHandle);

		RenderGraphResourceHandle TryGetRegisteredExternalResource(WeakPtr<RHI::RHIResource> resource);
		void RegisterExternalResource(WeakPtr<RHI::RHIResource> resource, RenderGraphResourceHandle handle);

		Vector<Image2DExtractionInfo> m_image2DExtractions;
		Vector<BufferExtractionInfo> m_bufferExtractions;

		Vector<Vector<MarkerFunction>> m_standaloneMarkers; // Pass -> Markers
		Vector<Vector<RenderGraphResourceHandle>> m_surrenderableResources; // Pass -> Resources
		Vector<Ref<RenderGraphPassNodeBase>> m_passNodes;
		Vector<Ref<RenderGraphResourceNodeBase>> m_resourceNodes;

		Vector<Vector<ResourceUsageInfo>> m_resourceBarriers; // Pass -> Transitions
		Vector<Vector<ResourceUsageInfo>> m_standaloneBarriers;
		
		vt::map<WeakPtr<RHI::RHIResource>, RenderGraphResourceHandle> m_registeredExternalResources;

		Vector<uint8_t*> m_temporaryAllocations;


		struct RegisteredImageView
		{
			ResourceHandle handle;
			RHI::ImageViewType viewType;
		};

		Vector<ResourceHandle> m_registeredBufferResources;
		Vector<RegisteredImageView> m_registeredImageResources;

		uint32_t m_passIndex = 0;
		uint32_t m_resourceIndex = 0;

		RefPtr<RHI::CommandBuffer> m_commandBuffer;
		WeakPtr<RHI::StorageBuffer> m_perPassConstantsBuffer;
		WeakPtr<RHI::UniformBuffer> m_renderGraphConstantsBuffer;
		ResourceHandle m_perPassConstantsBufferResourceHandle = Resource::Invalid;

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		ShaderRuntimeValidator m_runtimeShaderValidator;
#endif

		TransientResourceSystem m_transientResourceSystem;
		RenderContext m_renderContext;

		bool m_currentlyInBuilder = false;

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
		m_resourceBarriers.emplace_back();
		m_standaloneMarkers.emplace_back();

		m_currentlyInBuilder = true;
		Builder builder{ *this, newNode };
		createFunc(builder, newNode->data);

		AddRuntimeShaderValidationBuffers(builder);
		m_currentlyInBuilder = false;

		return newNode->data;
	}
}
