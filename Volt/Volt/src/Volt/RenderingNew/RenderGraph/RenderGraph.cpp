#include "vtpch.h"
#include "RenderGraph.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt
{
	RenderGraph::RenderGraph(Ref<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer), m_renderContext(commandBuffer)
	{
	}

	void RenderGraph::Compile()
	{
		///// Calculate Ref Count //////
		for (auto& pass : m_passNodes)
		{
			pass->refCount = static_cast<uint32_t>(pass->resourceWrites.size());

			for (const auto& handle : pass->resourceReads)
			{
				m_resourceNodes.at(handle)->refCount++;
			}

			for (const auto& handle : pass->resourceWrites)
			{
				m_resourceNodes.at(handle)->producer = pass;
			}
		}

		///// Cull Passes /////
		std::vector<Weak<RenderGraphResourceNodeBase>> unreferencedResources{};
		for (auto& node : m_resourceNodes)
		{
			if (node->refCount == 0)
			{
				unreferencedResources.push_back(node);
			}
		}

		while (!unreferencedResources.empty())
		{
			Weak<RenderGraphResourceNodeBase> unreferencedNode = unreferencedResources.back();
			unreferencedResources.pop_back();

			auto producer = unreferencedNode->producer;
			if (!producer || producer->hasSideEffect)
			{
				continue;
			}

			VT_CORE_ASSERT(producer->refCount > 0, "Ref count cannot be zero at this time!");

			producer->refCount--;
			if (producer->refCount == 0)
			{
				for (const auto& handle : producer->resourceReads)
				{
					auto node = m_resourceNodes.at(handle);
					node->refCount--;
					if (node->refCount == 0)
					{
						unreferencedResources.push_back(node);
					}
				}

				producer->isCulled = true;
			}
		}

		///// Find Resource usages /////
		for (auto& pass : m_passNodes)
		{
			if (pass->IsCulled())
			{
				continue;
			}

			for (const auto& handle : pass->resourceCreates)
			{
				m_resourceNodes.at(handle)->producer = pass;
			}

			for (const auto& handle : pass->resourceWrites)
			{
				m_resourceNodes.at(handle)->lastUsage = pass;
			}

			for (const auto& handle : pass->resourceReads)
			{
				m_resourceNodes.at(handle)->lastUsage = pass;
			}
		}

		std::vector<std::vector<RenderGraphResourceAccess>> resultAccesses;
		std::vector<std::unordered_map<RenderGraphResourceHandle, RenderGraphResourceAccess>> passAccesses; // Pass -> Resource -> Access info
		std::vector<int32_t> lastResourceAccess(m_resourceNodes.size(), -1);

		resultAccesses.resize(m_passNodes.size());
		passAccesses.resize(m_passNodes.size());
		m_resourceTransitions.resize(m_passNodes.size());

		///// Create resource barriers /////
		for (const auto& pass : m_passNodes)
		{
			if (pass->IsCulled())
			{
				continue;
			}

			for (const auto& resource : pass->resourceWrites)
			{
				int32_t lastAccessPassIndex = lastResourceAccess.at(resource);

				RHI::ResourceState oldState = RHI::ResourceState::Undefined;
				RHI::ResourceState newState = RHI::ResourceState::Undefined;

				if (lastAccessPassIndex != -1)
				{
					oldState = passAccesses[lastAccessPassIndex][resource].newState;
				}

				auto resourceNode = m_resourceNodes.at(resource);
				if (resourceNode->GetResourceType() == ResourceType::Image)
				{
					if (pass->isComputePass)
					{
						newState = RHI::ResourceState::UnorderedAccess;
					}
					else
					{
						newState = RHI::ResourceState::RenderTarget;
					}

				}
				else
				{
					newState = RHI::ResourceState::UnorderedAccess;
				}

				if (oldState == newState)
				{
					continue;
				}

				auto& newAccess = resultAccesses.at(pass->index).emplace_back();
				newAccess.oldState = oldState;
				newAccess.newState = newState;
				newAccess.resourceHandle = resource;

				passAccesses[pass->index][resource] = newAccess;
				lastResourceAccess[resource] = static_cast<int32_t>(pass->index);
			}

			for (const auto& resource : pass->resourceReads)
			{
				int32_t lastAccessPassIndex = lastResourceAccess.at(resource);

				RHI::ResourceState oldState = RHI::ResourceState::Undefined;
				RHI::ResourceState newState = RHI::ResourceState::Undefined;

				if (lastAccessPassIndex != -1)
				{
					oldState = passAccesses[lastAccessPassIndex][resource].newState;
				}

				auto resourceNode = m_resourceNodes.at(resource);
				if (resourceNode->GetResourceType() == ResourceType::Image)
				{
					if (pass->isComputePass)
					{
						newState = RHI::ResourceState::NonPixelShaderRead;
					}
					else
					{
						newState = RHI::ResourceState::PixelShaderRead;
					}
				}
				else
				{
					newState = RHI::ResourceState::NonPixelShaderRead | RHI::ResourceState::PixelShaderRead;
				}

				if (oldState == newState)
				{
					continue;
				}

				auto& newAccess = resultAccesses.at(pass->index).emplace_back();
				newAccess.oldState = oldState;
				newAccess.newState = newState;
				newAccess.resourceHandle = resource;

				passAccesses[pass->index][resource] = newAccess;
				lastResourceAccess[resource] = static_cast<int32_t>(pass->index);
			}
		}

		for (uint32_t passIndex = 0; auto& passTransitions : m_resourceTransitions)
		{
			if (!passTransitions.empty())
			{
				for (const auto& access : resultAccesses[passIndex])
				{
					passTransitions.emplace_back(access);
				}
			}
			else
			{
				passTransitions = resultAccesses[passIndex];
			}

			passIndex++;
		}
	}

	void RenderGraph::Execute()
	{
		m_commandBuffer->Begin();

		for (const auto& passNode : m_passNodes)
		{
			if (passNode->IsCulled())
			{
				continue;
			}

			if (!m_resourceTransitions.at(passNode->index).empty())
			{
				std::vector<RHI::ResourceBarrierInfo> barrierInfo{};

				for (const auto& transition : m_resourceTransitions.at(passNode->index))
				{
					auto& barrier = barrierInfo.emplace_back();
					barrier.oldState = transition.oldState;
					barrier.newState = transition.newState;
				}
			}

			{
				passNode->Execute(*this, m_renderContext);
			}
		}

		m_commandBuffer->End();
		m_commandBuffer->Execute();
	}

	RenderGraphResourceHandle RenderGraph::AddExternalImage2D(Ref<RHI::Image2D> image)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphTexture2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphTexture2D>>();
		node->handle = resourceHandle;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalBuffer(Ref<RHI::StorageBuffer> buffer)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));
	
		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphTexture2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphTexture2D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateImage3D(const RenderGraphImageDesc& textureDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphTexture3D>> node = CreateRef<RenderGraphResourceNode<RenderGraphTexture3D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;

		node->resourceInfo.description.usage = node->resourceInfo.description.usage | RHI::BufferUsage::StorageBuffer;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;

		node->resourceInfo.description.usage = node->resourceInfo.description.usage | RHI::BufferUsage::UniformBuffer;

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

	RenderGraphResourceHandle RenderGraph::Builder::AddExternalImage2D(Ref<RHI::Image2D> image)
	{
		return m_renderGraph.AddExternalImage2D(image);
	}

	RenderGraphResourceHandle RenderGraph::Builder::AddExternalBuffer(Ref<RHI::StorageBuffer> buffer)
	{
		return m_renderGraph.AddExternalBuffer(buffer);
	}

	RenderGraphResourceHandle RenderGraph::Builder::AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer)
	{
		return m_renderGraph.AddExternalUniformBuffer(buffer);
	}

	void RenderGraph::AddPass(std::string_view name, std::function<void(RenderGraph::Builder&)> createFunc, std::function<void(RenderContext&, const RenderGraphPassResources&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		struct Empty
		{
		};

		Ref<RenderGraphPassNode<Empty>> newNode = CreateRef<RenderGraphPassNode<Empty>>();
		newNode->name = name;
		newNode->executeFunction = [executeFunc](const Empty&, RenderContext& context, const RenderGraphPassResources& resources)
		{
			executeFunc(context, resources);
		};

		newNode->index = m_passIndex++;
		newNode->name = name;

		m_passNodes.push_back(newNode);
		m_resourceTransitions.emplace_back();

		Builder builder{ *this, newNode };
		createFunc(builder);
	}

	void RenderGraph::AddResourceTransition(RenderGraphResourceHandle resourceHandle, RHI::ResourceState newState)
	{
		RHI::ResourceState currentState = m_resourceNodes.at(resourceHandle)->currentState;
		m_resourceNodes.at(resourceHandle)->currentState = newState;

		if (m_resourceTransitions.size() <= static_cast<size_t>(m_passIndex))
		{
			m_resourceTransitions.resize(m_passIndex + 1);
		}

		auto& newAccess = m_resourceTransitions.at(m_passIndex).emplace_back();
		newAccess.oldState = currentState;
		newAccess.newState = newState;
		newAccess.resourceHandle = resourceHandle;
	}

	void RenderGraph::Builder::ReadResource(RenderGraphResourceHandle handle)
	{
		m_pass->resourceReads.emplace_back(handle);
	}

	void RenderGraph::Builder::WriteResource(RenderGraphResourceHandle handle)
	{
		if (!m_pass->CreatesResource(handle))
		{
			m_pass->resourceWrites.emplace_back(handle);
		}
	}

	void RenderGraph::Builder::SetIsComputePass()
	{
		m_pass->isComputePass = true;
	}

	void RenderGraph::Builder::SetHasSideEffect()
	{
		m_pass->hasSideEffect = true;
	}
}
