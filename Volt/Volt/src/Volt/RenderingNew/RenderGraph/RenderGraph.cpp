#include "vtpch.h"
#include "RenderGraph.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"

namespace Volt
{
	RenderGraph::RenderGraph(Ref<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	void RenderGraph::Compile()
	{
	}

	void RenderGraph::Execute()
	{
	}

	RenderGraphResourceHandle RenderGraph::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphTexture2D>> node{};
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateImage3D(const RenderGraphImageDesc& textureDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphTexture3D>> node{};
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node{};
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node{};
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	Ref<RHI::Image2D> RenderGraph::GetImage2D(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphTexture2D>>().resourceInfo;

		return m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
	}

	Ref<RHI::StorageBuffer> RenderGraph::GetBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		return m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
	}

	Ref<RHI::UniformBuffer> RenderGraph::GetUniformBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		return m_transientResourceSystem.AquireUniformBuffer(resourceHandle, bufferDesc.description);
	}

	RenderGraph::Builder::Builder(RenderGraph& renderGraph, Ref<RenderGraphPassNodeBase> pass)
		: m_renderGraph(renderGraph), m_pass(pass)
	{
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		return m_renderGraph.CreateImage2D(textureDesc);
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateImage3D(const RenderGraphImageDesc& textureDesc)
	{
		return m_renderGraph.CreateImage3D(textureDesc);
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		return m_renderGraph.CreateBuffer(bufferDesc);
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		return m_renderGraph.CreateUniformBuffer(bufferDesc);
	}

	void RenderGraph::AddPass(std::string_view name, std::function<void(RenderGraph::Builder&)> createFunc, std::function<void(const RenderGraphPassResources&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		struct Empty
		{};

		Ref<RenderGraphPassNode<Empty>> newNode = CreateRef<RenderGraphPassNode<Empty>>();
		newNode->name = name;
		newNode->executeFunction = [executeFunc](const Empty&, const RenderGraphPassResources& resources)
		{
			executeFunc(resources);
		};

		newNode->passIndex = m_passIndex++;
		newNode->name = name;

		m_passNodes.push_back(newNode);

		Builder builder{ *this, newNode };
		createFunc(builder);
	}

	void RenderGraph::Builder::ReadResource(RenderGraphResourceHandle handle)
	{
		m_pass.lock()->resourceReads.emplace_back(handle);
	}

	void RenderGraph::Builder::WriteResource(RenderGraphResourceHandle handle)
	{
		if (!m_pass.lock()->CreatesResource(handle))
		{
			m_pass.lock()->resourceWrites.emplace_back(handle);
		}
	}

	void RenderGraph::Builder::SetIsComputePass()
	{
		m_pass.lock()->isComputePass = true;
	}

	void RenderGraph::Builder::SetHasSideEffect()
	{
		m_pass.lock()->hasSideEffect = true;
	}
}
