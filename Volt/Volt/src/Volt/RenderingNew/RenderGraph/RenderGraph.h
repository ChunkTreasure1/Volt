#pragma once

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/RenderingNew/TransientResourceSystem/TransientResourceSystem.h" 

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

			void ReadResource(RenderGraphResourceHandle handle);
			void WriteResource(RenderGraphResourceHandle handle);
		
		private:
			RenderGraph& m_renderGraph;
			Weak<RenderGraphPassNodeBase> m_pass;
		};

		void Compile();
		void Execute();

		template<typename T>
		T& AddPass(std::string_view name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, const RenderGraphPassResources&)>&& executeFunc);
		void AddPass(std::string_view name, std::function<void(Builder&)> createFunc, std::function<void(const RenderGraphPassResources&)>&& executeFunc);

	private:
		friend class RenderGraphPassResources;
		friend class Builder;

		RenderGraphResourceHandle CreateImage2D(const RenderGraphImageDesc& textureDesc);
		RenderGraphResourceHandle CreateImage3D(const RenderGraphImageDesc& textureDesc);
		RenderGraphResourceHandle CreateBuffer(const RenderGraphBufferDesc& bufferDesc);
		RenderGraphResourceHandle CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc);

		Ref<RHI::Image2D> GetImage2D(const RenderGraphResourceHandle resourceHandle);
		//Ref<RHI::Image3D> GetImage3D(const RenderGraphResourceHandle resourceHandle); // #TODO: Implement Image3D first
		Ref<RHI::StorageBuffer> GetBuffer(const RenderGraphResourceHandle resourceHandle);
		Ref<RHI::UniformBuffer> GetUniformBuffer(const RenderGraphResourceHandle resourceHandle);

		std::vector<Ref<RenderGraphPassNodeBase>> m_passNodes;
		std::vector<Ref<RenderGraphResourceNodeBase>> m_resourceNodes;

		uint32_t m_passIndex = 0;
		RenderGraphResourceHandle m_resourceIndex = 0;

		Weak<RHI::CommandBuffer> m_commandBuffer;
		TransientResourceSystem m_transientResourceSystem;
	};

	template<typename T>
	inline T& RenderGraph::AddPass(std::string_view name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, const RenderGraphPassResources&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		
		Ref<RenderGraphPassNode<T>> newNode = CreateRef<RenderGraphPassNode<T>>();
		newNode->name = name;
		newNode->executeFunction = executeFunc;
		newNode->passIndex = m_passIndex++;
		newNode->name = name;

		m_passNodes.push_back(newNode);

		Builder builder{ *this, newNode };
		createFunc(builder, newNode->data);

		return newNode->data;
	}
}