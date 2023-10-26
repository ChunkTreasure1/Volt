#pragma once

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/RenderingNew/RenderGraph/RenderContext.h"
#include "Volt/RenderingNew/TransientResourceSystem/TransientResourceSystem.h" 
#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"

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

	class RenderGraph
	{
	public:
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

			void SetHasSideEffect();
			void SetIsComputePass();

			void ReadResource(RenderGraphResourceHandle handle, RHI::ResourceState forceState = RHI::ResourceState::Undefined);
			void WriteResource(RenderGraphResourceHandle handle, RHI::ResourceState forceState = RHI::ResourceState::Undefined);
		
		private:
			RenderGraph& m_renderGraph;
			Weak<RenderGraphPassNodeBase> m_pass;
		};

		void Compile();

		// NOTE: After calling Execute the RenderGraph object is no longer valid to use!
		void Execute();

		template<typename T>
		T& AddPass(std::string_view name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, RenderContext&, const RenderGraphPassResources&)>&& executeFunc);
		void AddPass(std::string_view name, std::function<void(Builder&)> createFunc, std::function<void(RenderContext&, const RenderGraphPassResources&)>&& executeFunc);

		void AddResourceTransition(RenderGraphResourceHandle resourceHandle, RHI::ResourceState newState);
		void AddResourceTransition(RHI::ResourceState oldState, RHI::ResourceState newState);

		RenderGraphResourceHandle AddExternalImage2D(Ref<RHI::Image2D> image);
		//RenderGraphResourceHandle AddExternalImage3D(Ref<RHI::Image3D> image);
		RenderGraphResourceHandle AddExternalBuffer(Ref<RHI::StorageBuffer> buffer);
		RenderGraphResourceHandle AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer);

	private:
		friend class RenderGraphPassResources;
		friend class Builder;
		friend class RenderGraphExecutionThread;

		void ExecuteInternal();
		void DestroyResources();
		void AllocateConstantsBuffer();

		RenderGraphResourceHandle CreateImage2D(const RenderGraphImageDesc& textureDesc);
		RenderGraphResourceHandle CreateImage3D(const RenderGraphImageDesc& textureDesc);
		RenderGraphResourceHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc);
		RenderGraphResourceHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc);

		ResourceHandle GetImage2D(const RenderGraphResourceHandle resourceHandle);
		//Ref<RHI::Image3D> GetImage3D(const RenderGraphResourceHandle resourceHandle); // #TODO: Implement Image3D first
		ResourceHandle GetBuffer(const RenderGraphResourceHandle resourceHandle);
		ResourceHandle GetUniformBuffer(const RenderGraphResourceHandle resourceHandle);

		Weak<RHI::RHIResource> GetResourceRaw(const RenderGraphResourceHandle resourceHandle);

		std::vector<Ref<RenderGraphPassNodeBase>> m_passNodes;
		std::vector<Ref<RenderGraphResourceNodeBase>> m_resourceNodes;
		std::vector<std::vector<RenderGraphResourceAccess>> m_resourceTransitions; // Pass -> Transitions
		
		std::vector<ResourceHandle> m_usedGlobalImage2DResourceHandles;
		std::vector<ResourceHandle> m_usedGlobalBufferResourceHandles;

		uint32_t m_passIndex = 0;
		RenderGraphResourceHandle m_resourceIndex = 0;

		Weak<RHI::CommandBuffer> m_commandBuffer;

		Weak<RHI::StorageBuffer> m_passConstantsBuffer;
		ResourceHandle m_passConstantsBufferResourceHandle = 0;

		TransientResourceSystem m_transientResourceSystem;
		RenderContext m_renderContext;
	};

	template<typename T>
	inline T& RenderGraph::AddPass(std::string_view name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, RenderContext&, const RenderGraphPassResources&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		
		Ref<RenderGraphPassNode<T>> newNode = CreateRef<RenderGraphPassNode<T>>();
		newNode->name = name;
		newNode->executeFunction = executeFunc;
		newNode->index = m_passIndex++;
		newNode->name = name;

		m_passNodes.push_back(newNode);
		m_resourceTransitions.emplace_back();

		Builder builder{ *this, newNode };
		createFunc(builder, newNode->data);

		return newNode->data;
	}
}
