#pragma once

#include "Volt/Rendering/RenderGraph/RenderGraphPass.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/Rendering/RenderGraph/RenderContext.h"
#include "Volt/Rendering/TransientResourceSystem/TransientResourceSystem.h" 

#include "Volt/Core/Base.h"

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
	};

	class RenderGraph
	{
	public:
		typedef std::function<void(Ref<RHI::CommandBuffer> commandBuffer)> MarkerFunction;
		typedef std::function<void(const uint64_t allocatedSize)> TotalAllocatedSizeCallback;

		RenderGraph(Ref<RHI::CommandBuffer> commandBuffer);
		~RenderGraph();

		class Builder
		{
		public:
			Builder(RenderGraph& renderGraph, Ref<RenderGraphPassNodeBase> pass);

			RenderGraphResourceHandle CreateImage2D(const RenderGraphImageDesc& textureDesc);
			RenderGraphResourceHandle CreateImage3D(const RenderGraphImageDesc& textureDesc);
			RenderGraphResourceHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc);
			RenderGraphResourceHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc);

			RenderGraphResourceHandle AddExternalImage2D(Ref<RHI::Image2D> image, bool trackGlobalResource = true);
			//RenderGraphResourceHandle AddExternalImage3D(Ref<RHI::Image3D> image, bool trackGlobalResource = true);
			RenderGraphResourceHandle AddExternalBuffer(Ref<RHI::StorageBuffer> buffer, bool trackGlobalResource = true);
			RenderGraphResourceHandle AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer, bool trackGlobalResource = true);

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

		template<typename T>
		T& AddPass(const std::string& name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, RenderContext&, const RenderGraphPassResources&)>&& executeFunc);
		void AddPass(const std::string& name, std::function<void(Builder&)> createFunc, std::function<void(RenderContext&, const RenderGraphPassResources&)>&& executeFunc);

		void AddMappedBufferUpload(RenderGraphResourceHandle bufferHandle, const void* data, const size_t size, std::string_view name);
		void AddResourceBarrier(RenderGraphResourceHandle resourceHandle, const RenderGraphBarrierInfo& barrierInfo);

		void QueueImage2DExtraction(RenderGraphResourceHandle resourceHandle, Ref<RHI::Image2D>& outImage);
		void QueueBufferExtraction(RenderGraphResourceHandle resourceHandle, Ref<RHI::StorageBuffer>& outBuffer);

		void BeginMarker(const std::string& markerName, const glm::vec4& markerColor = 1.f);
		void EndMarker();

		void SetTotalAllocatedSizeCallback(TotalAllocatedSizeCallback&& callback);

		RenderGraphResourceHandle AddExternalImage2D(Ref<RHI::Image2D> image, bool trackGlobalResource = true);
		//RenderGraphResourceHandle AddExternalImage3D(Ref<RHI::Image3D> image, bool trackGlobalResource = true);
		RenderGraphResourceHandle AddExternalBuffer(Ref<RHI::StorageBuffer> buffer, bool trackGlobalResource = true);
		RenderGraphResourceHandle AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer, bool trackGlobalResource = true);

		RenderGraphResourceHandle CreateImage2D(const RenderGraphImageDesc& textureDesc);
		RenderGraphResourceHandle CreateImage3D(const RenderGraphImageDesc& textureDesc);
		RenderGraphResourceHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc);
		RenderGraphResourceHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc);

		ResourceHandle GetImage2D(const RenderGraphResourceHandle resourceHandle, const uint32_t mip = 0, const uint32_t layer = 0);
		ResourceHandle GetImage2DArray(const RenderGraphResourceHandle resourceHandle, const uint32_t mip = 0);
		//Ref<RHI::Image3D> GetImage3D(const RenderGraphResourceHandle resourceHandle); // #TODO: Implement Image3D first
		ResourceHandle GetBuffer(const RenderGraphResourceHandle resourceHandle);
		ResourceHandle GetUniformBuffer(const RenderGraphResourceHandle resourceHandle);

	private:
		friend class RenderGraphPassResources;
		friend class Builder;
		friend class RenderGraphExecutionThread;
		friend class RenderContext;

		inline static constexpr RenderGraphResourceHandle INVALID_RESOURCE_HANDLE = std::numeric_limits<RenderGraphResourceHandle>::max();

		void ExecuteInternal();
		void InsertStandaloneMarkers(const uint32_t passIndex);
		void DestroyResources();
		void AllocateConstantsBuffer();
		void ExtractResources();

		struct Image2DExtractionInfo
		{
			RenderGraphResourceHandle resourceHandle;
			Ref<RHI::Image2D>* outImagePtr = nullptr;
		};

		struct BufferExtractionInfo
		{
			RenderGraphResourceHandle resourceHandle;
			Ref<RHI::StorageBuffer>* outBufferPtr = nullptr;
		};

		Weak<RHI::ImageView> GetImage2DView(const RenderGraphResourceHandle resourceHandle);
		Weak<RHI::Image2D> GetImage2DRaw(const RenderGraphResourceHandle resourceHandle);
		Weak<RHI::StorageBuffer> GetBufferRaw(const RenderGraphResourceHandle resourceHandle);
		Weak<RHI::RHIResource> GetResourceRaw(const RenderGraphResourceHandle resourceHandle);

		RenderGraphResourceHandle TryGetRegisteredExternalResource(Weak<RHI::RHIResource> resource);
		void RegisterExternalResource(Weak<RHI::RHIResource> resource, RenderGraphResourceHandle handle);

		std::vector<Image2DExtractionInfo> m_image2DExtractions;
		std::vector<BufferExtractionInfo> m_bufferExtractions;

		std::vector<std::vector<MarkerFunction>> m_standaloneMarkers; // Pass -> Markers
		std::vector<std::vector<RenderGraphResourceHandle>> m_surrenderableResources; // Pass -> Resources
		std::vector<Ref<RenderGraphPassNodeBase>> m_passNodes;
		std::vector<Ref<RenderGraphResourceNodeBase>> m_resourceNodes;

		std::vector<std::vector<ResourceUsageInfo>> m_resourceBarriers; // Pass -> Transitions
		std::vector<std::vector<ResourceUsageInfo>> m_standaloneBarriers;
		
		std::unordered_map<Weak<RHI::RHIResource>, RenderGraphResourceHandle> m_registeredExternalResources;

		std::vector<uint8_t*> m_temporaryAllocations;


		struct RegisteredImageView
		{
			ResourceHandle handle;
			RHI::ImageViewType viewType;
		};

		std::vector<ResourceHandle> m_registeredBufferResources;
		std::vector<RegisteredImageView> m_registeredImageResources;

		uint32_t m_passIndex = 0;
		RenderGraphResourceHandle m_resourceIndex = 0;

		Weak<RHI::CommandBuffer> m_commandBuffer;
		Weak<RHI::StorageBuffer> m_passConstantsBuffer;
		ResourceHandle m_passConstantsBufferResourceHandle = Resource::Invalid;

		TransientResourceSystem m_transientResourceSystem;
		RenderContext m_renderContext;

		bool m_currentlyInBuilder = false;

		TotalAllocatedSizeCallback m_totalAllocatedSizeCallback;
	};

	template<typename T>
	inline T& RenderGraph::AddPass(const std::string& name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, RenderContext&, const RenderGraphPassResources&)>&& executeFunc)
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
		m_currentlyInBuilder = false;

		return newNode->data;
	}
}
