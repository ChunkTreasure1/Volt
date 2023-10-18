#include "vtpch.h"
#include "RenderGraph.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"

#include "Volt/Core/Profiling.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Images/ImageUtility.h>

namespace Volt
{
	RenderGraph::RenderGraph(Ref<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer), m_renderContext(commandBuffer)
	{
	}

	void RenderGraph::Compile()
	{
		VT_PROFILE_FUNCTION();

		///// Calculate Ref Count //////
		for (auto& pass : m_passNodes)
		{
			pass->refCount = static_cast<uint32_t>(pass->resourceWrites.size());

			for (const auto& access : pass->resourceReads)
			{
				m_resourceNodes.at(access.handle)->refCount++;
			}

			for (const auto& access : pass->resourceWrites)
			{
				m_resourceNodes.at(access.handle)->producer = pass;
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
				for (const auto& access : producer->resourceReads)
				{
					auto node = m_resourceNodes.at(access.handle);
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

			for (const auto& access : pass->resourceWrites)
			{
				m_resourceNodes.at(access.handle)->lastUsage = pass;
			}

			for (const auto& access : pass->resourceReads)
			{
				m_resourceNodes.at(access.handle)->lastUsage = pass;
			}
		}

		std::vector<std::vector<RenderGraphResourceAccess>> resultAccesses;
		std::vector<std::unordered_map<RenderGraphResourceHandle, RenderGraphResourceAccess>> passAccesses; // Pass -> Resource -> Access info
		std::vector<int32_t> lastResourceAccess(m_resourceNodes.size(), -1);

		resultAccesses.resize(m_passNodes.size());
		passAccesses.resize(m_passNodes.size());

		if (m_resourceTransitions.size() < m_passNodes.size())
		{
			m_resourceTransitions.resize(m_passNodes.size());
		}

		///// Create resource barriers /////
		for (const auto& pass : m_passNodes)
		{
			if (pass->IsCulled())
			{
				continue;
			}

			auto writeResourceFunc = [&](const RenderGraphPassResourceAccess& access)
			{
				int32_t lastAccessPassIndex = lastResourceAccess.at(access.handle);

				RHI::ResourceState oldState = RHI::ResourceState::Undefined;
				RHI::ResourceState newState = RHI::ResourceState::Undefined;

				if (lastAccessPassIndex != -1)
				{
					oldState = passAccesses[lastAccessPassIndex][access.handle].newState;
				}

				auto resourceNode = m_resourceNodes.at(access.handle);
				if (access.state != RHI::ResourceState::Undefined)
				{
					newState = access.state;
				}
				else
				{
					if (resourceNode->GetResourceType() == ResourceType::Image2D || resourceNode->GetResourceType() == ResourceType::Image3D)
					{
						if (pass->isComputePass)
						{
							newState = RHI::ResourceState::UnorderedAccess;
						}
						else
						{
							if (resourceNode->GetResourceType() == ResourceType::Image2D)
							{
								RenderGraphResourceNode<RenderGraphImage2D>& image2DNode = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>();
								if (RHI::Utility::IsDepthFormat(image2DNode.resourceInfo.description.format) || RHI::Utility::IsStencilFormat(image2DNode.resourceInfo.description.format))
								{
									newState = RHI::ResourceState::DepthWrite;
								}
								else
								{
									newState = RHI::ResourceState::RenderTarget;
								}
							}
							else
							{
								newState = RHI::ResourceState::RenderTarget;
							}
						}
					}
					else
					{
						newState = RHI::ResourceState::UnorderedAccess;
					}
				}

				if (oldState == newState)
				{
					return;
				}

				auto& newAccess = resultAccesses.at(pass->index).emplace_back();
				newAccess.oldState = oldState;
				newAccess.newState = newState;
				newAccess.resourceHandle = access.handle;

				passAccesses[pass->index][access.handle] = newAccess;
				lastResourceAccess[access.handle] = static_cast<int32_t>(pass->index);
			};

			for (const auto& resource : pass->resourceCreates)
			{
				writeResourceFunc(RenderGraphPassResourceAccess{ RHI::ResourceState::Undefined, resource });
			}

			for (const auto& resource : pass->resourceWrites)
			{
				writeResourceFunc(resource);
			}

			for (const auto& access : pass->resourceReads)
			{
				int32_t lastAccessPassIndex = lastResourceAccess.at(access.handle);

				RHI::ResourceState oldState = RHI::ResourceState::Undefined;
				RHI::ResourceState newState = RHI::ResourceState::Undefined;

				if (lastAccessPassIndex != -1)
				{
					oldState = passAccesses[lastAccessPassIndex][access.handle].newState;
				}

				auto resourceNode = m_resourceNodes.at(access.handle);
				if (access.state != RHI::ResourceState::Undefined)
				{
					newState = access.state;
				}
				else
				{
					if (resourceNode->GetResourceType() == ResourceType::Image2D || resourceNode->GetResourceType() == ResourceType::Image3D)
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
				}


				if (oldState == newState)
				{
					continue;
				}

				auto& newAccess = resultAccesses.at(pass->index).emplace_back();
				newAccess.oldState = oldState;
				newAccess.newState = newState;
				newAccess.resourceHandle = access.handle;

				passAccesses[pass->index][access.handle] = newAccess;
				lastResourceAccess[access.handle] = static_cast<int32_t>(pass->index);
			}
		}

		for (uint32_t passIndex = 0; auto & passTransitions : m_resourceTransitions)
		{
			if (!passTransitions.empty())
			{
				if (resultAccesses.size() > static_cast<size_t>(passIndex))
				{
					for (const auto& access : resultAccesses[passIndex])
					{
						passTransitions.emplace_back(access);
					}
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
		RenderGraphExecutionThread::ExecuteRenderGraph(std::move(*this));
	}

	RenderGraphResourceHandle RenderGraph::AddExternalImage2D(Ref<RHI::Image2D> image)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.description.format = image->GetFormat();

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalBuffer(Ref<RHI::StorageBuffer> buffer)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.isExternal = true;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.isExternal = true;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		return resourceHandle;
	}

	void RenderGraph::ExecuteInternal()
	{
		VT_PROFILE_FUNCTION();

		m_commandBuffer->Begin();

		for (const auto& passNode : m_passNodes)
		{
			if (passNode->IsCulled())
			{
				continue;
			}

			m_commandBuffer->BeginMarker(passNode->name, { 1.f, 1.f, 1.f, 1.f });

			if (!m_resourceTransitions.at(passNode->index).empty())
			{
				std::vector<RHI::ResourceBarrierInfo> barrierInfos{};

				for (const auto& transition : m_resourceTransitions.at(passNode->index))
				{
					auto& barrier = barrierInfos.emplace_back();
					barrier.oldState = transition.oldState;
					barrier.newState = transition.newState;
					barrier.resource = GetResourceRaw(transition.resourceHandle);

					m_resourceNodes.at(transition.resourceHandle)->currentState = transition.newState;
				}

				m_commandBuffer->ResourceBarrier(barrierInfos);
			}

			{
				VT_PROFILE_SCOPE(passNode->name.data());
				passNode->Execute(*this, m_renderContext);
			}

			m_commandBuffer->EndMarker();
		}

		// Add the barriers specified after last pass
		if (m_resourceTransitions.size() > m_passIndex)
		{
			std::vector<RHI::ResourceBarrierInfo> barrierInfos{};

			for (const auto& transition : m_resourceTransitions.at(m_passIndex))
			{
				auto& barrier = barrierInfos.emplace_back();
				barrier.oldState = transition.oldState;
				barrier.newState = transition.newState;
				barrier.resource = GetResourceRaw(transition.resourceHandle);
			}

			m_commandBuffer->ResourceBarrier(barrierInfos);
		}

		m_commandBuffer->End();
		m_commandBuffer->Execute();
	}

	RenderGraphResourceHandle RenderGraph::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
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

	Weak<RHI::Image2D> RenderGraph::GetImage2D(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		return m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
	}

	Weak<RHI::StorageBuffer> RenderGraph::GetBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		return m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
	}

	Weak<RHI::UniformBuffer> RenderGraph::GetUniformBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		return m_transientResourceSystem.AquireUniformBuffer(resourceHandle, bufferDesc.description);
	}

	Weak<RHI::RHIResource> RenderGraph::GetResourceRaw(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);

		Weak<RHI::RHIResource> result{};

		switch (resourceNode->GetResourceType())
		{
			case ResourceType::Image2D:
			{
				const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;
				result = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);

				break;
			}

			case ResourceType::Image3D:
				break;

			case ResourceType::Buffer:
			{
				const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;
				result = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);

				break;
			}

			case ResourceType::UniformBuffer:
			{
				const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphUniformBuffer>>().resourceInfo;
				result = m_transientResourceSystem.AquireUniformBuffer(resourceHandle, bufferDesc.description);

				break;
			}
		}

		return result;
	}

	RenderGraph::Builder::Builder(RenderGraph& renderGraph, Ref<RenderGraphPassNodeBase> pass)
		: m_renderGraph(renderGraph), m_pass(pass)
	{
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		const auto resourceId = m_renderGraph.CreateImage2D(textureDesc);
		m_pass->resourceCreates.emplace_back(resourceId);

		return resourceId;
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateImage3D(const RenderGraphImageDesc& textureDesc)
	{
		const auto resourceId = m_renderGraph.CreateImage3D(textureDesc);
		m_pass->resourceCreates.emplace_back(resourceId);

		return resourceId;
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		const auto resourceId = m_renderGraph.CreateBuffer(bufferDesc);
		m_pass->resourceCreates.emplace_back(resourceId);

		return resourceId;
	}

	RenderGraphResourceHandle RenderGraph::Builder::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		const auto resourceId = m_renderGraph.CreateUniformBuffer(bufferDesc);
		m_pass->resourceCreates.emplace_back(resourceId);

		return resourceId;
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

	void RenderGraph::AddResourceTransition(RHI::ResourceState oldState, RHI::ResourceState newState)
	{
		if (m_resourceTransitions.size() <= static_cast<size_t>(m_passIndex))
		{
			m_resourceTransitions.resize(m_passIndex + 1);
		}

		auto& newAccess = m_resourceTransitions.at(m_passIndex).emplace_back();
		newAccess.oldState = oldState;
		newAccess.newState = newState;
	}

	void RenderGraph::Builder::ReadResource(RenderGraphResourceHandle handle, RHI::ResourceState forceState)
	{
		m_pass->resourceReads.emplace_back(forceState, handle);
	}

	void RenderGraph::Builder::WriteResource(RenderGraphResourceHandle handle, RHI::ResourceState forceState)
	{
		if (!m_pass->CreatesResource(handle))
		{
			m_pass->resourceWrites.emplace_back(forceState, handle);
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
