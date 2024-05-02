#include "vtpch.h"
#include "RenderGraph.h"

#include "Volt/Rendering/RenderGraph/RenderGraphPass.h"
#include "Volt/Rendering/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/Rendering/RenderGraph/RenderGraphCommon.h"

#include "Volt/Rendering/Renderer.h"

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Images/ImageView.h>

#include <CoreUtilities/Profiling/Profiling.h>

namespace Volt
{
	namespace Utility
	{
		inline static bool IsSame(const RHI::ResourceBarrierInfo& lhs, const RHI::ResourceBarrierInfo& rhs, RHI::BarrierType type)
		{
			bool result = false;

			if (type == RHI::BarrierType::Image)
			{
				result = lhs.imageBarrier().dstStage == rhs.imageBarrier().dstStage &&
					lhs.imageBarrier().dstAccess == rhs.imageBarrier().dstAccess &&
					lhs.imageBarrier().dstLayout == rhs.imageBarrier().dstLayout;
			}
			else if (type == RHI::BarrierType::Buffer)
			{
				result = lhs.bufferBarrier().dstStage == rhs.bufferBarrier().dstStage &&
					lhs.bufferBarrier().dstAccess == rhs.bufferBarrier().dstAccess;
			}

			return result;
		}

		inline static void SetupForcedState(const RenderGraphResourceState state, ResourceUsageInfo& usageInfo, ResourceType resourceType)
		{
			if (state == RenderGraphResourceState::IndirectArgument)
			{
				usageInfo.accessInfo.type = RHI::BarrierType::Buffer;
				usageInfo.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::IndirectArgument;
				usageInfo.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::Indirect;
			}
			else if (state == RenderGraphResourceState::IndexBuffer)
			{
				usageInfo.accessInfo.type = RHI::BarrierType::Buffer;
				usageInfo.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::IndexBuffer;
				usageInfo.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::IndexInput;
			}
			else if (state == RenderGraphResourceState::VertexBuffer)
			{
				usageInfo.accessInfo.type = RHI::BarrierType::Buffer;
				usageInfo.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::VertexBuffer;
				usageInfo.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::VertexShader;
			}
			else if (state == RenderGraphResourceState::TransferDestination)
			{
				if (resourceType == ResourceType::Image2D)
				{
					usageInfo.accessInfo.type = RHI::BarrierType::Image;
					usageInfo.accessInfo.imageBarrier().dstAccess = RHI::BarrierAccess::TransferDestination;
					usageInfo.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::Copy;
					usageInfo.accessInfo.imageBarrier().dstLayout = RHI::ImageLayout::TransferDestination;
				}
			}
		}

		inline static void SetupBufferBarrier(RHI::BufferBarrier& bufferBarrier, Weak<RHI::RHIResource> resource)
		{
			bufferBarrier.resource = resource;
			bufferBarrier.size = resource->GetByteSize();
		}
	}

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

		// Find surrenderable resources
		m_surrenderableResources.resize(m_passIndex);

		for (const auto& resource : m_resourceNodes)
		{
			if (!resource->lastUsage || resource->isExternal)
			{
				continue;
			}

			m_surrenderableResources[resource->lastUsage->index].emplace_back(resource->handle);
		}

		if (m_resourceBarriers.size() < m_passNodes.size())
		{
			m_resourceBarriers.resize(m_passNodes.size());
		}

		struct LastPassUsageInfo
		{
			bool resourceWasWritten = false;
			RenderGraphResourceAccess accessInfo{};
		};

		///// Create resource barriers /////
		std::vector<std::vector<ResourceUsageInfo>> resourceUsages;
		resourceUsages.resize(m_resourceIndex);

		for (const auto& pass : m_passNodes)
		{
			if (!pass->IsCulled())
			{
				auto writeResourceFunc = [&](const RenderGraphPassResourceAccess& access) -> void
				{
					ResourceUsageInfo previousUsageInfo{};
					bool hasPreviousUsage = false;

					if (!resourceUsages.at(access.handle).empty())
					{
						hasPreviousUsage = true;
						previousUsageInfo = resourceUsages.at(access.handle).back();
					}

					ResourceUsageInfo usage;
					usage.passIndex = pass->index;
					usage.resourceHandle = access.handle;

					const auto resource = m_resourceNodes.at(access.handle);

					// Setup previous usage

					if (hasPreviousUsage)
					{
						if (resource->GetResourceType() == ResourceType::Image2D || resource->GetResourceType() == ResourceType::Image3D)
						{
							usage.accessInfo.imageBarrier().srcStage = previousUsageInfo.accessInfo.imageBarrier().dstStage;
							usage.accessInfo.imageBarrier().srcAccess = previousUsageInfo.accessInfo.imageBarrier().dstAccess;
							usage.accessInfo.imageBarrier().srcLayout = previousUsageInfo.accessInfo.imageBarrier().dstLayout;
						}
						else if (resource->GetResourceType() == ResourceType::Buffer)
						{
							usage.accessInfo.bufferBarrier().srcStage = previousUsageInfo.accessInfo.bufferBarrier().dstStage;
							usage.accessInfo.bufferBarrier().srcAccess = previousUsageInfo.accessInfo.bufferBarrier().dstAccess;
						}
					}
					else
					{
						if (resource->GetResourceType() == ResourceType::Image2D || resource->GetResourceType() == ResourceType::Image3D)
						{
							// If it's an unused external image, use it's own layout
							if (resource->isExternal)
							{
								auto imageResource = GetImage2DRaw(access.handle);
								previousUsageInfo.accessInfo.imageBarrier().dstLayout = imageResource->GetImageLayout();
							}
							else
							{
								previousUsageInfo.accessInfo.imageBarrier().dstLayout = RHI::ImageLayout::Undefined;
							}

							previousUsageInfo.accessInfo.type = RHI::BarrierType::Image;
							previousUsageInfo.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::All;
							previousUsageInfo.accessInfo.imageBarrier().dstAccess = RHI::BarrierAccess::None;
						}
						else if (resource->GetResourceType() == ResourceType::Buffer)
						{
							previousUsageInfo.accessInfo.type = RHI::BarrierType::Buffer;
							previousUsageInfo.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::All;
							previousUsageInfo.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::None;
						}
					}

					// Setup new usage
					if (access.forcedState != RenderGraphResourceState::None)
					{
						Utility::SetupForcedState(access.forcedState, usage, resource->GetResourceType());
					}
					else
					{
						if (resource->GetResourceType() == ResourceType::Image2D || resource->GetResourceType() == ResourceType::Image3D)
						{
							usage.accessInfo.type = RHI::BarrierType::Image;

							if (pass->isComputePass)
							{
								usage.accessInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderWrite;
								usage.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
								usage.accessInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderWrite;
							}
							else
							{
								if (resource->GetResourceType() == ResourceType::Image2D)
								{
									RenderGraphResourceNode<RenderGraphImage2D>& image2DNode = resource->As<RenderGraphResourceNode<RenderGraphImage2D>>();

									if (RHI::Utility::IsDepthFormat(image2DNode.resourceInfo.description.format) || RHI::Utility::IsStencilFormat(image2DNode.resourceInfo.description.format))
									{
										usage.accessInfo.imageBarrier().dstAccess = RHI::BarrierAccess::DepthStencilWrite;
										usage.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::DepthStencil;
										usage.accessInfo.imageBarrier().dstLayout = RHI::ImageLayout::DepthStencilWrite;
									}
									else
									{
										usage.accessInfo.imageBarrier().dstAccess = RHI::BarrierAccess::RenderTarget;
										usage.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::RenderTarget;
										usage.accessInfo.imageBarrier().dstLayout = RHI::ImageLayout::RenderTarget;
									}
								}
							}

						}
						else if (resource->GetResourceType() == ResourceType::Buffer)
						{
							usage.accessInfo.type = RHI::BarrierType::Buffer;
							usage.accessInfo.bufferBarrier().offset = 0;

							if (pass->isComputePass)
							{
								usage.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::ShaderWrite;
								usage.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::ComputeShader;
							}
							else
							{
								usage.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
								usage.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader;
							}
						}
					}

					if (hasPreviousUsage && Utility::IsSame(previousUsageInfo.accessInfo, usage.accessInfo, usage.accessInfo.type))
					{
						// Resource doesn't need a barrier
						return;
					}

					resourceUsages.at(access.handle).emplace_back(usage);
				};

				auto readResourceFunc = [&](const RenderGraphPassResourceAccess& access)
				{
					ResourceUsageInfo previousUsageInfo{};
					bool hasPreviousUsage = false;

					if (!resourceUsages.at(access.handle).empty())
					{
						hasPreviousUsage = true;
						previousUsageInfo = resourceUsages.at(access.handle).back();
					}

					ResourceUsageInfo usage;
					usage.passIndex = pass->index;
					usage.resourceHandle = access.handle;

					const auto resource = m_resourceNodes.at(access.handle);

					// If it's an unused external image, use it's own layout
					if (!hasPreviousUsage && resource->isExternal)
					{
						if (resource->GetResourceType() == ResourceType::Image2D)
						{
							auto imageResource = GetImage2DRaw(access.handle);
							previousUsageInfo.accessInfo.imageBarrier().dstLayout = imageResource->GetImageLayout();
						}
					}

					// Setup previous usage
					if (resource->GetResourceType() == ResourceType::Image2D || resource->GetResourceType() == ResourceType::Image3D)
					{
						usage.accessInfo.imageBarrier().srcStage = previousUsageInfo.accessInfo.imageBarrier().dstStage;
						usage.accessInfo.imageBarrier().srcAccess = previousUsageInfo.accessInfo.imageBarrier().dstAccess;
						usage.accessInfo.imageBarrier().srcLayout = previousUsageInfo.accessInfo.imageBarrier().dstLayout;
					}
					else if (resource->GetResourceType() == ResourceType::Buffer)
					{
						usage.accessInfo.bufferBarrier().srcStage = previousUsageInfo.accessInfo.bufferBarrier().dstStage;
						usage.accessInfo.bufferBarrier().srcAccess = previousUsageInfo.accessInfo.bufferBarrier().dstAccess;
					}

					// Setup new usage
					if (access.forcedState != RenderGraphResourceState::None)
					{
						Utility::SetupForcedState(access.forcedState, usage, resource->GetResourceType());
					}
					else
					{
						if (resource->GetResourceType() == ResourceType::Image2D || resource->GetResourceType() == ResourceType::Image3D)
						{
							usage.accessInfo.type = RHI::BarrierType::Image;
							usage.accessInfo.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
							usage.accessInfo.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;

							if (pass->isComputePass)
							{
								usage.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::ComputeShader;
							}
							else
							{
								usage.accessInfo.imageBarrier().dstStage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader;
							}
						}
						else if (resource->GetResourceType() == ResourceType::Buffer)
						{
							usage.accessInfo.type = RHI::BarrierType::Buffer;
							usage.accessInfo.bufferBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
							usage.accessInfo.bufferBarrier().offset = 0;

							if (pass->isComputePass)
							{
								usage.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::ComputeShader;
							}
							else
							{
								usage.accessInfo.bufferBarrier().dstStage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader;
							}
						}
					}

					if (hasPreviousUsage && Utility::IsSame(previousUsageInfo.accessInfo, usage.accessInfo, usage.accessInfo.type))
					{
						// Resource doesn't need a barrier
						return;
					}

					resourceUsages.at(access.handle).emplace_back(usage);
				};

				for (const auto& write : pass->resourceCreates)
				{
					writeResourceFunc({ RenderGraphResourceState::None, write });
				}

				for (const auto& write : pass->resourceWrites)
				{
					writeResourceFunc(write);
				}

				for (const auto& read : pass->resourceReads)
				{
					readResourceFunc(read);
				}
			}

			if (m_standaloneBarriers.size() > static_cast<size_t>(pass->index) && !m_standaloneBarriers.at(pass->index).empty())
			{
				for (auto& barrier : m_standaloneBarriers.at(pass->index))
				{
					if (barrier.accessInfo.type == RHI::BarrierType::Image)
					{
						if (!resourceUsages.at(barrier.resourceHandle).empty())
						{
							const auto& previousUsage = resourceUsages.at(barrier.resourceHandle).back();

							barrier.accessInfo.imageBarrier().srcAccess = previousUsage.accessInfo.imageBarrier().dstAccess;
							barrier.accessInfo.imageBarrier().srcStage = previousUsage.accessInfo.imageBarrier().dstStage;
							barrier.accessInfo.imageBarrier().srcLayout = previousUsage.accessInfo.imageBarrier().dstLayout;

						}
					}
					else if (barrier.accessInfo.type == RHI::BarrierType::Buffer)
					{
						if (!resourceUsages.at(barrier.resourceHandle).empty())
						{
							const auto& previousUsage = resourceUsages.at(barrier.resourceHandle).back();

							barrier.accessInfo.bufferBarrier().srcAccess = previousUsage.accessInfo.bufferBarrier().dstAccess;
							barrier.accessInfo.bufferBarrier().srcStage = previousUsage.accessInfo.bufferBarrier().dstStage;
						}
					}

					resourceUsages.at(barrier.resourceHandle).emplace_back(barrier);
				}
			}
		}

		for (const auto& resourceUsage : resourceUsages)
		{
			for (const auto& usage : resourceUsage)
			{
				if (usage.isPassSpecificUsage)
				{
					m_resourceBarriers.at(usage.passIndex).emplace_back(usage);
				}
			}
		}
	}

	void RenderGraph::Execute()
	{
		RenderGraphExecutionThread::ExecuteRenderGraph(std::move(*this));
	}

	RenderGraphResourceHandle RenderGraph::AddExternalImage2D(Ref<RHI::Image2D> image, bool trackGlobalResource)
	{
		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(image); registeredHandle != INVALID_RESOURCE_HANDLE)
		{
			return registeredHandle;
		}

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.trackGlobalResource = trackGlobalResource;
		node->resourceInfo.description.format = image->GetFormat();

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		RegisterExternalResource(image, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalBuffer(Ref<RHI::StorageBuffer> buffer, bool trackGlobalResource)
	{
		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(buffer); registeredHandle != INVALID_RESOURCE_HANDLE)
		{
			return registeredHandle;
		}

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.trackGlobalResource = trackGlobalResource;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		RegisterExternalResource(buffer, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	RenderGraphResourceHandle RenderGraph::AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer, bool trackGlobalResource)
	{
		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(buffer); registeredHandle != INVALID_RESOURCE_HANDLE)
		{
			return registeredHandle;
		}

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.trackGlobalResource = trackGlobalResource;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, std::reinterpret_pointer_cast<RHI::RHIResource>(buffer));

		RegisterExternalResource(buffer, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	void RenderGraph::ExecuteInternal()
	{
		VT_PROFILE_FUNCTION();

		AllocateConstantsBuffer();
		m_renderContext.SetPassConstantsBuffer(m_passConstantsBuffer);
		m_renderContext.SetRenderGraphInstance(this);

		m_commandBuffer->Begin();

		for (const auto& passNode : m_passNodes)
		{
			if (passNode->IsCulled())
			{
				InsertStandaloneMarkers(passNode->index);
				continue;
			}

			m_commandBuffer->BeginMarker(passNode->name, { 1.f, 1.f, 1.f, 1.f });

			if (!m_resourceBarriers.at(passNode->index).empty())
			{
				std::vector<RHI::ResourceBarrierInfo> barrierInfos{};

				for (const auto& transition : m_resourceBarriers.at(passNode->index))
				{
					auto& barrier = barrierInfos.emplace_back(transition.accessInfo);

					if (barrier.type == RHI::BarrierType::Image)
					{
						barrier.imageBarrier().resource = GetResourceRaw(transition.resourceHandle);
						m_resourceNodes.at(transition.resourceHandle)->currentState = { transition.accessInfo.imageBarrier().dstStage, transition.accessInfo.imageBarrier().dstAccess, transition.accessInfo.imageBarrier().dstLayout };
					}
					else if (barrier.type == RHI::BarrierType::Buffer)
					{
						Utility::SetupBufferBarrier(barrier.bufferBarrier(), GetResourceRaw(transition.resourceHandle));
						m_resourceNodes.at(transition.resourceHandle)->currentState = { transition.accessInfo.bufferBarrier().dstStage, transition.accessInfo.bufferBarrier().dstAccess, RHI::ImageLayout::Undefined };
					}
				}

				m_commandBuffer->ResourceBarrier(barrierInfos);
			}

			{
				VT_PROFILE_SCOPE(passNode->name.data());
				m_renderContext.SetCurrentPassIndex(passNode);

				passNode->Execute(*this, m_renderContext);
			}

			m_commandBuffer->EndMarker();

			InsertStandaloneMarkers(passNode->index);

			for (const auto& resourceHandle : m_surrenderableResources.at(passNode->index))
			{
				const auto resource = m_resourceNodes.at(resourceHandle);
				m_transientResourceSystem.SurrenderResource(resourceHandle, resource->hash);
			}
		}

		// Add the barriers specified after last pass
		if (!m_standaloneBarriers.empty())
		{
			std::vector<RHI::ResourceBarrierInfo> barrierInfos{};

			for (const auto& barriers : m_standaloneBarriers)
			{
				for (const auto& graphBarrier : barriers)
				{
					auto& barrier = barrierInfos.emplace_back(graphBarrier.accessInfo);

					if (graphBarrier.accessInfo.type == RHI::BarrierType::Image)
					{
						barrier.imageBarrier().resource = GetResourceRaw(graphBarrier.resourceHandle);
					}
					else if (graphBarrier.accessInfo.type == RHI::BarrierType::Buffer)
					{
						Utility::SetupBufferBarrier(barrier.bufferBarrier(), GetResourceRaw(graphBarrier.resourceHandle));
					}
				}
			}

			m_commandBuffer->ResourceBarrier(barrierInfos);
		}

		m_renderContext.UploadConstantsData();

		m_commandBuffer->End();

		GlobalResourceManager::RenderGraphUpdate();

		m_commandBuffer->Execute();

		if (m_totalAllocatedSizeCallback)
		{
			m_totalAllocatedSizeCallback(m_transientResourceSystem.GetTotalAllocatedSize());
		}

		DestroyResources();
	}

	void RenderGraph::InsertStandaloneMarkers(const uint32_t passIndex)
	{
		for (const auto& marker : m_standaloneMarkers.at(passIndex))
		{
			marker(m_commandBuffer);
		}
	}

	void RenderGraph::DestroyResources()
	{
		for (const auto& resource : m_usedGlobalImageResourceHandles)
		{
			if (resource.specialization == ResourceSpecialization::Texture1D)
			{
				GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::Texture1D>(resource.handle);
			}
			else if (resource.specialization == ResourceSpecialization::Texture2D)
			{
				GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(resource.handle);
			}
			else if (resource.specialization == ResourceSpecialization::Texture3D)
			{
				GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::Texture3D>(resource.handle);
			}
			else if (resource.specialization == ResourceSpecialization::Texture2DArray)
			{
				GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::Texture2DArray>(resource.handle);
			}
			else if (resource.specialization == ResourceSpecialization::TextureCube)
			{
				GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::TextureCube>(resource.handle);
			}
		}

		for (const auto& resource : m_usedGlobalBufferResourceHandles)
		{
			if (resource.specialization == ResourceSpecialization::UniformBuffer)
			{
				GlobalResourceManager::UnregisterResource<RHI::StorageBuffer, ResourceSpecialization::UniformBuffer>(resource.handle);
			}
			else
			{
				GlobalResourceManager::UnregisterResource<RHI::StorageBuffer>(resource.handle);
			}
		}

		GlobalResourceManager::UnregisterResource<RHI::StorageBuffer>(m_passConstantsBufferResourceHandle);

		ExtractResources();

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
		m_passConstantsBufferResourceHandle = GlobalResourceManager::RegisterResource<RHI::StorageBuffer>(m_passConstantsBuffer);
	}

	void RenderGraph::ExtractResources()
	{
		for (const auto& imageExtractionData : m_image2DExtractions)
		{
			if (imageExtractionData.outImagePtr == nullptr)
			{
				continue;
			}

			auto rawImage = GetImage2DRaw(imageExtractionData.resourceHandle);
			*imageExtractionData.outImagePtr = rawImage;
		}

		for (const auto& bufferExtractionData : m_bufferExtractions)
		{
			if (bufferExtractionData.outBufferPtr == nullptr)
			{
				continue;
			}

			auto rawBuffer = GetBufferRaw(bufferExtractionData.resourceHandle);
			*bufferExtractionData.outBufferPtr = rawBuffer;
		}
	}

	RenderGraphResourceHandle RenderGraph::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		VT_CORE_ASSERT(textureDesc.width > 0 && textureDesc.height > 0, "Width and height must not be zero!");

		RenderGraphResourceHandle resourceHandle = m_resourceIndex++;
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;
		node->isExternal = !m_currentlyInBuilder;
		node->hash = Utility::GetHashFromImageDesc(textureDesc);

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
		node->hash = Utility::GetHashFromImageDesc(textureDesc);

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
		node->hash = Utility::GetHashFromBufferDesc(bufferDesc);

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
		node->hash = Utility::GetHashFromBufferDesc(bufferDesc);

		node->resourceInfo.description.usage = node->resourceInfo.description.usage | RHI::BufferUsage::StorageBuffer;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	Weak<RHI::ImageView> RenderGraph::GetImage2DView(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		if (!view->IsSwapchainView())
		{
			ResourceHandle handle = Resource::Invalid;
			ResourceSpecialization specialization = ResourceSpecialization::None;

			if (view->GetViewType() == RHI::ImageViewType::View2D)
			{
				handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(view);
				specialization = ResourceSpecialization::Texture2D;
			}
			else if (view->GetViewType() == RHI::ImageViewType::View2DArray)
			{
				handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2DArray>(view);
				specialization = ResourceSpecialization::Texture2DArray;
			}
			else if (view->GetViewType() == RHI::ImageViewType::ViewCube)
			{
				handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::TextureCube>(view);
				specialization = ResourceSpecialization::TextureCube;
			}

			if (imageDesc.trackGlobalResource)
			{
				m_usedGlobalImageResourceHandles.insert(GlobalResourceInfo{ handle, specialization });
			}
		}

		return view;
	}

	Weak<RHI::Image2D> RenderGraph::GetImage2DRaw(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		// #TODO_Ivar: Move this section to it's own function
		if (!view->IsSwapchainView())
		{
			ResourceHandle handle = Resource::Invalid;
			ResourceSpecialization specialization = ResourceSpecialization::None;

			if (view->GetViewType() == RHI::ImageViewType::View2D)
			{
				handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(view);
				specialization = ResourceSpecialization::Texture2D;
			}
			else if (view->GetViewType() == RHI::ImageViewType::View2DArray)
			{
				handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2DArray>(view);
				specialization = ResourceSpecialization::Texture2DArray;
			}
			else if (view->GetViewType() == RHI::ImageViewType::ViewCube)
			{
				handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::TextureCube>(view);
				specialization = ResourceSpecialization::TextureCube;
			}

			if (imageDesc.trackGlobalResource)
			{
				m_usedGlobalImageResourceHandles.insert({ handle, specialization });
			}
		}

		return image;
	}

	ResourceHandle RenderGraph::GetImage2D(const RenderGraphResourceHandle resourceHandle, const uint32_t mip, const uint32_t layer)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetView(mip, layer);

		VT_ENSURE(!view->IsSwapchainView());

		ResourceHandle handle = Resource::Invalid;
		ResourceSpecialization specialization = ResourceSpecialization::None;

		if (view->GetViewType() == RHI::ImageViewType::View2D)
		{
			handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(view);
			specialization = ResourceSpecialization::Texture2D;
		}
		else if (view->GetViewType() == RHI::ImageViewType::View2DArray)
		{
			handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2DArray>(view);
			specialization = ResourceSpecialization::Texture2DArray;
		}
		else if (view->GetViewType() == RHI::ImageViewType::ViewCube)
		{
			handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::TextureCube>(view);
			specialization = ResourceSpecialization::TextureCube;
		}

		if (imageDesc.trackGlobalResource)
		{
			m_usedGlobalImageResourceHandles.insert(GlobalResourceInfo{ handle, specialization });
		}

		return handle;
	}

	ResourceHandle RenderGraph::GetImage2DArray(const RenderGraphResourceHandle resourceHandle, const uint32_t mip)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetArrayView(mip);

		VT_ENSURE(!view->IsSwapchainView());

		ResourceHandle handle = Resource::Invalid;
		ResourceSpecialization specialization = ResourceSpecialization::None;

		if (view->GetViewType() == RHI::ImageViewType::View2D)
		{
			handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(view);
			specialization = ResourceSpecialization::Texture2D;
		}
		else if (view->GetViewType() == RHI::ImageViewType::View2DArray)
		{
			handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2DArray>(view);
			specialization = ResourceSpecialization::Texture2DArray;
		}
		else if (view->GetViewType() == RHI::ImageViewType::ViewCube)
		{
			handle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::TextureCube>(view);
			specialization = ResourceSpecialization::TextureCube;
		}

		if (imageDesc.trackGlobalResource)
		{
			m_usedGlobalImageResourceHandles.insert(GlobalResourceInfo{ handle, specialization });
		}

		return handle;
	}

	ResourceHandle RenderGraph::GetBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::StorageBuffer>(buffer);

		if (bufferDesc.trackGlobalResource)
		{
			m_usedGlobalBufferResourceHandles.insert(GlobalResourceInfo{ handle, ResourceSpecialization::None });
		}

		return handle;
	}

	Weak<RHI::StorageBuffer> RenderGraph::GetBufferRaw(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::StorageBuffer>(buffer);

		if (bufferDesc.trackGlobalResource)
		{
			m_usedGlobalBufferResourceHandles.insert(GlobalResourceInfo{ handle, ResourceSpecialization::None });
		}

		return buffer;
	}

	ResourceHandle RenderGraph::GetUniformBuffer(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphUniformBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = GlobalResourceManager::RegisterResource<RHI::StorageBuffer, ResourceSpecialization::UniformBuffer>(buffer);

		if (bufferDesc.trackGlobalResource)
		{
			m_usedGlobalBufferResourceHandles.insert(GlobalResourceInfo{ handle, ResourceSpecialization::UniformBuffer });
		}

		return handle;
	}

	Weak<RHI::RHIResource> RenderGraph::GetResourceRaw(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

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
				result = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);

				break;
			}
		}

		return result;
	}

	RenderGraphResourceHandle RenderGraph::TryGetRegisteredExternalResource(Weak<RHI::RHIResource> resource)
	{
		if (m_registeredExternalResources.contains(resource))
		{
			return m_registeredExternalResources.at(resource);
		}

		return INVALID_RESOURCE_HANDLE;
	}

	void RenderGraph::RegisterExternalResource(Weak<RHI::RHIResource> resource, RenderGraphResourceHandle handle)
	{
		m_registeredExternalResources[resource] = handle;
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
		m_resourceBarriers.emplace_back();
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
			context.MappedBufferUpload(bufferHandle, tempData, size);
		};

		m_passNodes.push_back(newNode);
		m_resourceBarriers.emplace_back();
		m_standaloneMarkers.emplace_back();

		m_temporaryAllocations.emplace_back(tempData);
	}

	void RenderGraph::AddResourceBarrier(RenderGraphResourceHandle resourceHandle, const RenderGraphBarrierInfo& barrierInfo)
	{
		if (m_standaloneBarriers.size() < m_passIndex)
		{
			m_standaloneBarriers.resize(m_passIndex);
		}

		if (m_passIndex == 0)
		{
			m_standaloneBarriers.resize(1);
		}

		const uint32_t currentIndex = m_passIndex > 0 ? m_passIndex - 1 : 0;
		auto& newBarrier = m_standaloneBarriers.at(currentIndex).emplace_back();
		newBarrier.passIndex = currentIndex;
		newBarrier.resourceHandle = resourceHandle;

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);

		// Source values is being set up during the compile step

		if (resourceNode->GetResourceType() == ResourceType::Image2D || resourceNode->GetResourceType() == ResourceType::Image3D)
		{
			newBarrier.accessInfo.type = RHI::BarrierType::Image;
			newBarrier.accessInfo.imageBarrier().srcStage = RHI::BarrierStage::None;
			newBarrier.accessInfo.imageBarrier().srcAccess = RHI::BarrierAccess::None;
			newBarrier.accessInfo.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;

			newBarrier.accessInfo.imageBarrier().dstStage = barrierInfo.dstStage;
			newBarrier.accessInfo.imageBarrier().dstAccess = barrierInfo.dstAccess;
			newBarrier.accessInfo.imageBarrier().dstLayout = barrierInfo.dstLayout;
		}
		else if (resourceNode->GetResourceType() == ResourceType::Buffer)
		{
			newBarrier.accessInfo.type = RHI::BarrierType::Buffer;
			newBarrier.accessInfo.bufferBarrier().srcStage = RHI::BarrierStage::None;
			newBarrier.accessInfo.bufferBarrier().srcAccess = RHI::BarrierAccess::None;

			newBarrier.accessInfo.bufferBarrier().dstStage = barrierInfo.dstStage;
			newBarrier.accessInfo.bufferBarrier().dstAccess = barrierInfo.dstAccess;
		}

		newBarrier.isPassSpecificUsage = false;
	}

	void RenderGraph::QueueImage2DExtraction(RenderGraphResourceHandle resourceHandle, Ref<RHI::Image2D>& outImage)
	{
		m_image2DExtractions.emplace_back(resourceHandle, &outImage);
	}

	void RenderGraph::QueueBufferExtraction(RenderGraphResourceHandle resourceHandle, Ref<RHI::StorageBuffer>& outBuffer)
	{
		m_bufferExtractions.emplace_back(resourceHandle, &outBuffer);
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

	RenderGraphResourceHandle RenderGraph::Builder::AddExternalImage2D(Ref<RHI::Image2D> image, bool trackGlobalResource)
	{
		return m_renderGraph.AddExternalImage2D(image, trackGlobalResource);
	}

	RenderGraphResourceHandle RenderGraph::Builder::AddExternalBuffer(Ref<RHI::StorageBuffer> buffer, bool trackGlobalResource)
	{
		return m_renderGraph.AddExternalBuffer(buffer, trackGlobalResource);
	}

	RenderGraphResourceHandle RenderGraph::Builder::AddExternalUniformBuffer(Ref<RHI::UniformBuffer> buffer, bool trackGlobalResource)
	{
		return m_renderGraph.AddExternalUniformBuffer(buffer, trackGlobalResource);
	}

	void RenderGraph::Builder::ReadResource(RenderGraphResourceHandle handle, RenderGraphResourceState forceState)
	{
		m_pass->resourceReads.emplace_back(forceState, handle);
	}

	void RenderGraph::Builder::WriteResource(RenderGraphResourceHandle handle, RenderGraphResourceState forceState)
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
