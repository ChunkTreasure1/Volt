#include "vtpch.h"
#include "RenderGraph.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphCommon.h"

#include "Volt/RenderingNew/RendererNew.h"

#include "Volt/Core/Profiling.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

namespace Volt
{
	RenderGraph::RenderGraph(Ref<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer), m_renderContext(commandBuffer)
	{
	}

	RenderGraph::~RenderGraph()
	{
	}

	void RenderGraph::Compile()
	{
		VT_PROFILE_FUNCTION();

		// Validate standalone markers
		size_t markerCount = 0;
		for (const auto& markers : m_standaloneMarkers)
		{
			markerCount += markers.size();
		}

		markerCount;
		VT_CORE_ASSERT(markerCount % 2u == 0, "There must be a EndMarker for every BeginMarker!");

		///// Calculate Ref Count //////
		for (auto& pass : m_passNodes)
		{
			pass->refCount = static_cast<uint32_t>(pass->resourceWrites.size());
			pass->refCount += static_cast<uint32_t>(pass->resourceCreates.size());

			for (const auto& access : pass->resourceReads)
			{
				m_resourceNodes.at(access.handle)->refCount++;
			}

			for (const auto& handle : pass->resourceCreates)
			{
				m_resourceNodes.at(handle)->producer = pass;
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

			if (unreferencedNode->isExternal)
			{
				continue;
			}

			auto producer = unreferencedNode->producer;
			VT_CORE_ASSERT(producer, "Node should always have a producer!");

			if (producer->hasSideEffect)
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

		m_resourcesToSurrender.resize(m_passIndex);

		for (const auto& resource : m_resourceNodes)
		{
			if (!resource->lastUsage)
			{
				continue;
			}

			// Last pass resources
			if (resource->lastUsage->index == m_passIndex - 1)
			{
				return;
			}

			m_resourcesToSurrender.at(resource->lastUsage->index).emplace_back(resource->handle);
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

		struct LastPassUsageInfo
		{
			bool resourceWasUsed = false;
			RenderGraphResourceAccess accessInfo{};
		};

		///// Create resource barriers /////
		for (const auto& pass : m_passNodes)
		{
			if (pass->IsCulled())
			{
				continue;
			}

			auto writeResourceFunc = [&](const RenderGraphPassResourceAccess& access) -> LastPassUsageInfo
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

				LastPassUsageInfo lastUsageInfo{};

				if (oldState == RHI::ResourceState::UnorderedAccess || oldState == RHI::ResourceState::DepthWrite || oldState == RHI::ResourceState::RenderTarget)
				{
					lastUsageInfo.resourceWasUsed = true;
					lastUsageInfo.accessInfo.oldState = oldState;
					lastUsageInfo.accessInfo.newState = newState;
				}

				if (oldState == newState)
				{
					return lastUsageInfo;
				}

				auto& newAccess = resultAccesses.at(pass->index).emplace_back();
				newAccess.oldState = oldState;
				newAccess.newState = newState;
				newAccess.resourceHandle = access.handle;

				passAccesses[pass->index][access.handle] = newAccess;
				lastResourceAccess[access.handle] = static_cast<int32_t>(pass->index);

				return lastUsageInfo;
			};

			LastPassUsageInfo previousDependenciesInfo{};

			for (const auto& resource : pass->resourceCreates)
			{
				writeResourceFunc(RenderGraphPassResourceAccess{ RHI::ResourceState::Undefined, resource });
			}

			for (const auto& resource : pass->resourceWrites)
			{
				const auto lastUsageInfo = writeResourceFunc(resource);
				if (lastUsageInfo.resourceWasUsed)
				{
					previousDependenciesInfo.resourceWasUsed = true;
					previousDependenciesInfo.accessInfo.oldState |= lastUsageInfo.accessInfo.oldState;
					previousDependenciesInfo.accessInfo.newState |= lastUsageInfo.accessInfo.newState;
				}
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

				if (oldState == RHI::ResourceState::UnorderedAccess || oldState == RHI::ResourceState::DepthWrite || oldState == RHI::ResourceState::RenderTarget)
				{
					previousDependenciesInfo.resourceWasUsed = true;
					previousDependenciesInfo.accessInfo.oldState |= oldState;
					previousDependenciesInfo.accessInfo.newState |= newState;
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

			if (previousDependenciesInfo.resourceWasUsed)
			{
				auto& execBarrier = resultAccesses.at(pass->index).emplace_back();
				execBarrier.oldState = previousDependenciesInfo.accessInfo.oldState;
				execBarrier.newState = previousDependenciesInfo.accessInfo.newState;
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
			else if (passIndex < static_cast<uint32_t>(resultAccesses.size()))
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

	RenderGraphResourceHandle RenderGraph::AddExternalImage2D(Ref<RHI::Image2D> image, bool trackGlobalResource)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.trackGlobalResource = trackGlobalResource;
		node->resourceInfo.description.format = image->GetFormat();

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalBuffer(Ref<RHI::StorageBuffer> buffer, bool trackGlobalResource)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.trackGlobalResource = trackGlobalResource;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer, bool trackGlobalResource)
	{
		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.trackGlobalResource = trackGlobalResource;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		return resourceHandle;
	}

	void RenderGraph::ExecuteInternal()
	{
		VT_PROFILE_FUNCTION();

		AllocateConstantsBuffer();
		m_renderContext.SetPassConstantsBuffer(m_passConstantsBuffer);

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

					if (barrier.resource)
					{
						m_resourceNodes.at(transition.resourceHandle)->currentState = transition.newState;
					}
				}

				m_commandBuffer->ResourceBarrier(barrierInfos);
			}

			{
				VT_PROFILE_SCOPE(passNode->name.data());
				m_renderContext.SetCurrentPassIndex(passNode->index);

				passNode->Execute(*this, m_renderContext);
			}

			m_commandBuffer->EndMarker();

			for (const auto& marker : m_standaloneMarkers.at(passNode->index))
			{
				marker(m_commandBuffer);
			}

			// Surrender resources
			for (const auto& resource : m_resourcesToSurrender.at(passNode->index))
			{
				m_transientResourceSystem.SurrenderResource(resource);
			}
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

		m_renderContext.UploadConstantsData();

		m_commandBuffer->End();

		GlobalResourceManager::Update();

		m_commandBuffer->Execute();

		if (m_totalAllocatedSizeCallback)
		{
			m_totalAllocatedSizeCallback(m_transientResourceSystem.GetTotalAllocatedSize());
		}

		DestroyResources();
	}

	void RenderGraph::DestroyResources()
	{
		RenderGraphExecutionThread::DestroyRenderGraphResource([usedImage2Ds = m_usedGlobalImage2DResourceHandles, usedBuffers = m_usedGlobalBufferResourceHandles, passConstantsBuffer = m_passConstantsBufferResourceHandle]()
		{
			for (const auto& resource : usedImage2Ds)
			{
				GlobalResourceManager::UnregisterResource<RHI::ImageView>(resource);
			}

			for (const auto& resource : usedBuffers)
			{
				GlobalResourceManager::UnregisterResource<RHI::StorageBuffer>(resource);
			}

			GlobalResourceManager::UnregisterResource<RHI::StorageBuffer>(passConstantsBuffer);
		});

		for (const auto& alloc : m_temporaryAllocations)
		{
			delete[] alloc;
		}

		m_temporaryAllocations.clear();
	}

	void RenderGraph::AllocateConstantsBuffer()
	{
		RenderGraphBufferDesc desc{};
		desc.size = m_passIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE;
		desc.usage = RHI::BufferUsage::StorageBuffer;
		desc.memoryUsage = RHI::MemoryUsage::CPUToGPU;
		desc.name = "Render Graph Constants";

		m_passConstantsBuffer = m_transientResourceSystem.AquireBuffer(m_resourceIndex++, desc);
		m_passConstantsBufferResourceHandle = GlobalResourceManager::RegisterResource(m_passConstantsBuffer);
	}

	RenderGraphResourceHandle RenderGraph::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		VT_CORE_ASSERT(textureDesc.width > 0 && textureDesc.height > 0, "Width and height must not be zero!");

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;
		node->isExternal = !m_currentlyInBuilder;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateImage3D(const RenderGraphImageDesc& textureDesc)
	{
		VT_CORE_ASSERT(textureDesc.width > 0 && textureDesc.height > 0, "Width and height must not be zero!");

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphTexture3D>> node = CreateRef<RenderGraphResourceNode<RenderGraphTexture3D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;
		node->isExternal = !m_currentlyInBuilder;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		VT_CORE_ASSERT(bufferDesc.size > 0, "Size must not be zero!");

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;
		node->isExternal = !m_currentlyInBuilder;

		node->resourceInfo.description.usage = node->resourceInfo.description.usage | RHI::BufferUsage::StorageBuffer;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		VT_CORE_ASSERT(bufferDesc.size > 0, "Size must not be zero!");

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;
		node->isExternal = !m_currentlyInBuilder;

		node->resourceInfo.description.usage = node->resourceInfo.description.usage | RHI::BufferUsage::UniformBuffer;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	Weak<RHI::ImageView> RenderGraph::GetImage2DView(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::ImageView>(image->GetView());

		m_usedGlobalImage2DResourceHandles.insert(handle);

		return image->GetView();
	}

	Weak<RHI::Image2D> RenderGraph::GetImage2DRaw(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::ImageView>(image->GetView());

		m_usedGlobalImage2DResourceHandles.insert(handle);

		return image;
	}

	ResourceHandle RenderGraph::GetImage2D(const RenderGraphResourceHandle resourceHandle, const uint32_t mip, const uint32_t layer)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::ImageView>(image->GetView(mip, layer));

		if (imageDesc.trackGlobalResource)
		{
			m_usedGlobalImage2DResourceHandles.insert(handle);
		}

		return handle;
	}

	ResourceHandle RenderGraph::GetBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::StorageBuffer>(buffer);

		if (bufferDesc.trackGlobalResource)
		{
			m_usedGlobalBufferResourceHandles.insert(handle);
		}

		return handle;
	}

	Weak<RHI::StorageBuffer> RenderGraph::GetBufferRaw(const RenderGraphResourceHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::StorageBuffer>(buffer);

		if (bufferDesc.trackGlobalResource)
		{
			m_usedGlobalBufferResourceHandles.insert(handle);
		}

		return buffer;
	}

	ResourceHandle RenderGraph::GetUniformBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		//const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		//const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		return Resource::Invalid;//m_transientResourceSystem.AquireUniformBuffer(resourceHandle, bufferDesc.description);
	}

	Weak<RHI::RHIResource> RenderGraph::GetResourceRaw(const RenderGraphResourceHandle resourceHandle)
	{
		if (resourceHandle == std::numeric_limits<RenderGraphResourceHandle>::max())
		{
			return {};
		}

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

	void RenderGraph::AddPass(const std::string& name, std::function<void(RenderGraph::Builder&)> createFunc, std::function<void(RenderContext&, const RenderGraphPassResources&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		struct Empty
		{
		};

		Ref<RenderGraphPassNode<Empty>> newNode = CreateRef<RenderGraphPassNode<Empty>>();
		newNode->name = name;
		newNode->index = m_passIndex++;
		newNode->executeFunction = [executeFunc](const Empty&, RenderContext& context, const RenderGraphPassResources& resources)
		{
			executeFunc(context, resources);
		};

		m_passNodes.push_back(newNode);
		m_resourceTransitions.emplace_back();
		m_standaloneMarkers.emplace_back();

		Builder builder{ *this, newNode };
		createFunc(builder);
	}

	void RenderGraph::AddMappedBufferUpload(RenderGraphResourceHandle bufferHandle, const void* data, const size_t size, std::string_view name)
	{
		struct Empty
		{
		};

		uint8_t* tempData = new uint8_t[size];
		memcpy_s(tempData, size, data, size);

		Ref<RenderGraphPassNode<Empty>> newNode = CreateRef<RenderGraphPassNode<Empty>>();
		newNode->name = name;
		newNode->index = m_passIndex++;

		Builder tempBuilder{ *this, newNode };
		tempBuilder.WriteResource(bufferHandle);

		newNode->executeFunction = [tempData, size, bufferHandle](const Empty&, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto buffer = resources.GetBufferRaw(bufferHandle);
			uint8_t* mappedPtr = buffer->Map<uint8_t>();
			memcpy_s(mappedPtr, size, tempData, size);
			buffer->Unmap();
		};

		m_passNodes.push_back(newNode);
		m_resourceTransitions.emplace_back();
		m_standaloneMarkers.emplace_back();

		m_temporaryAllocations.emplace_back(tempData);
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

	void RenderGraph::BeginMarker(const std::string& markerName, const glm::vec4& markerColor)
	{
		m_standaloneMarkers[m_passIndex - 1].emplace_back([markerName, markerColor](Ref<RHI::CommandBuffer> commandBuffer)
			{
				commandBuffer->BeginMarker(markerName, { markerColor.x, markerColor.y, markerColor.z, markerColor.w });
			});
	}

	void RenderGraph::EndMarker()
	{
		m_standaloneMarkers[m_passIndex - 1].emplace_back([](Ref<RHI::CommandBuffer> commandBuffer)
			{
				commandBuffer->EndMarker();
			});
	}

	void RenderGraph::SetTotalAllocatedSizeCallback(TotalAllocatedSizeCallback&& callback)
	{
		m_totalAllocatedSizeCallback = std::move(callback);
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
