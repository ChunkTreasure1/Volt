#include "rcpch.h"
#include "RenderGraph.h"

#include "RenderCore/RenderGraph/RenderGraphCommon.h"
#include "RenderCore/RenderGraph/RenderGraphExecutionThread.h"
#include "RenderCore/RenderGraph/RenderGraphPass.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "RenderCore/RenderGraph/GPUReadbackBuffer.h"

#include "RenderCore/Resources/BindlessResourcesManager.h"

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Images/Image3D.h>
#include <RHIModule/Images/ImageUtility.h>
#include <RHIModule/Images/ImageView.h>
#include <RHIModule/Utility/ResourceUtility.h>
#include <RHIModule/Synchronization/Fence.h>

#include <JobSystem/JobSystem.h>

#include <CoreUtilities/EnumUtils.h>
#include <CoreUtilities/Profiling/Profiling.h>

namespace Volt
{
	namespace Utility
	{
		inline void SetupForcedState(const RenderGraphResourceState forcedState, Ref<RenderGraphResourceNodeBase> resource, RHI::ResourceState& outState)
		{
			if (forcedState == RenderGraphResourceState::IndirectArgument)
			{
				outState.access = RHI::BarrierAccess::IndirectArgument;
				outState.stage = RHI::BarrierStage::DrawIndirect;
			}
			else if (forcedState == RenderGraphResourceState::IndexBuffer)
			{
				outState.access = RHI::BarrierAccess::IndexBuffer;
				outState.stage = RHI::BarrierStage::IndexInput;
			}
			else if (forcedState == RenderGraphResourceState::VertexBuffer)
			{
				outState.access = RHI::BarrierAccess::VertexBuffer;
				outState.stage = RHI::BarrierStage::VertexInput;
			}
			else if (forcedState == RenderGraphResourceState::CopyDest)
			{
				outState.access = RHI::BarrierAccess::CopyDest;
				outState.stage = RHI::BarrierStage::Copy;
				outState.layout = RHI::ImageLayout::CopyDest;
			}
			else if (forcedState == RenderGraphResourceState::CopySource)
			{
				outState.access = RHI::BarrierAccess::CopySource;
				outState.stage = RHI::BarrierStage::Copy;
				outState.layout = RHI::ImageLayout::CopySource;
			}
			else if (forcedState == RenderGraphResourceState::Clear)
			{
				if (resource->GetResourceType() == ResourceType::Buffer || resource->GetResourceType() == ResourceType::UniformBuffer)
				{
					outState.stage = RHI::BarrierStage::Clear;
				}
				else if (resource->GetResourceType() == ResourceType::Image2D)
				{
					RenderGraphResourceNode<RenderGraphImage2D>& image2DNode = resource->As<RenderGraphResourceNode<RenderGraphImage2D>>();
					if (RHI::Utility::IsDepthFormat(image2DNode.resourceInfo.description.format))
					{
						outState.access = RHI::BarrierAccess::DepthStencilWrite;
						outState.stage = RHI::BarrierStage::Clear;
						outState.layout = RHI::ImageLayout::DepthStencilWrite;
					}
					else
					{
						outState.access = RHI::BarrierAccess::RenderTarget;
						outState.stage = RHI::BarrierStage::Clear;
						outState.layout = RHI::ImageLayout::RenderTarget;
					}
				}
				else if (resource->GetResourceType() == ResourceType::Image3D)
				{
					outState.access = RHI::BarrierAccess::RenderTarget;
					outState.stage = RHI::BarrierStage::Clear;
					outState.layout = RHI::ImageLayout::RenderTarget;
				}
			}
		}

		inline static void SetupBufferBarrier(RHI::BufferBarrier& bufferBarrier, WeakPtr<RHI::RHIResource> resource)
		{
			bufferBarrier.resource = resource;
			bufferBarrier.size = resource->GetByteSize();
		}

		inline bool IsReadAfterRead(RHI::ResourceState srcState, RHI::ResourceState dstState)
		{
			if (!EnumValueContainsFlag(dstState.access, srcState.access))
			{
				return false;
			}

			if (!EnumValueContainsFlag(dstState.layout, srcState.layout))
			{
				return false;
			}

			if (!EnumValueContainsFlag(dstState.stage, srcState.stage))
			{
				return false;
			}

			return true;
		}

		template<typename T>
		inline T GetValueAsHandle(uint32_t val)
		{
			return *reinterpret_cast<T*>(&val);
		}

		template<typename T>
		inline T UpcastHandle(RenderGraphResourceHandle handle)
		{
			return *reinterpret_cast<T*>(&handle);
		}
	}

	RenderGraph::RenderGraph(RefPtr<RHI::CommandBuffer> commandBuffer)
		: m_commandBuffer(commandBuffer), m_renderContext(commandBuffer)
	{
		InitializeRuntimeShaderValidator();
	}

	RenderGraph::~RenderGraph()
	{
	}

	void RenderGraph::Compile()
	{
		VT_PROFILE_FUNCTION();

#ifndef VT_DIST
		// Validate standalone markers
		[[maybe_unused]] size_t markerCount = 0;
		for (const auto& markers : m_standaloneMarkers)
		{
			markerCount += markers.size();
		}

		VT_ASSERT_MSG(markerCount % 2u == 0, "There must be a EndMarker for every BeginMarker!");
#endif

		///// Calculate Ref Count //////
		for (auto& pass : m_passNodes)
		{
			pass->refCount = static_cast<uint32_t>(pass->resourceWrites.size()) + static_cast<uint32_t>(pass->resourceCreates.size());

			for (const auto& access : pass->resourceReads)
			{
				m_resourceNodes.at(access.handle.Get())->refCount++;
			}

			for (const auto& access : pass->resourceCreates)
			{
				m_resourceNodes.at(access.handle.Get())->producer = pass;
			}

			for (const auto& access : pass->resourceWrites)
			{
				m_resourceNodes.at(access.handle.Get())->producer = pass;
			}
		}

		///// Cull Passes /////
		Vector<Weak<RenderGraphResourceNodeBase>> unreferencedResources{};
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

			if (unreferencedNode->isExternal || unreferencedNode->isGlobal)
			{
				continue;
			}

			auto producer = unreferencedNode->producer;
			VT_ASSERT_MSG(producer, "Node should always have a producer!");

			if (producer->hasSideEffect)
			{
				continue;
			}

			VT_ASSERT_MSG(producer->refCount > 0, "Ref count cannot be zero at this time!");

			producer->refCount--;
			if (producer->refCount == 0)
			{
				for (const auto& access : producer->resourceReads)
				{
					auto node = m_resourceNodes.at(access.handle.Get());
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

			for (const auto& access : pass->resourceCreates)
			{
				m_resourceNodes.at(access.handle.Get())->producer = pass;
			}

			for (const auto& access : pass->resourceWrites)
			{
				m_resourceNodes.at(access.handle.Get())->lastUsage = pass;
			}

			for (const auto& access : pass->resourceReads)
			{
				m_resourceNodes.at(access.handle.Get())->lastUsage = pass;
			}
		}

		///// Find surrenerable resources /////
		m_surrenderableResources.resize(m_passIndex);

		for (const auto& resource : m_resourceNodes)
		{
			if (!resource->lastUsage || resource->isExternal)
			{
				continue;
			}

			m_surrenderableResources[resource->lastUsage->index].emplace_back(resource->handle);
		}

		///// Setup resource barriers /////

		// Keep track of all resources usages
		Vector<Vector<RHI::ResourceState>> resourceUsages;
		resourceUsages.resize(m_resourceIndex);

		auto createResourceFunc = [&](Ref<RenderGraphPassNodeBase> currentPass, const RenderGraphPassResourceAccess& access) -> void
		{
			const auto& resourceNode = m_resourceNodes.at(access.handle.Get());
			const auto resourceType = resourceNode->GetResourceType();

			RHI::ResourceState& newState = resourceUsages.at(access.handle.Get()).emplace_back();
			RHI::BarrierType barrierType = RHI::BarrierType::None;

			if (resourceType == ResourceType::Image2D || resourceType == ResourceType::Image3D)
			{
				if (currentPass->isComputePass)
				{
					newState.access = RHI::BarrierAccess::ShaderWrite;
					newState.stage = RHI::BarrierStage::ComputeShader;
					newState.layout = RHI::ImageLayout::ShaderWrite;
				}
				else
				{
					RenderGraphResourceNode<RenderGraphImage2D>& image2DNode = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>();

					if (RHI::Utility::IsDepthFormat(image2DNode.resourceInfo.description.format))
					{
						newState.access = RHI::BarrierAccess::DepthStencilWrite;
						newState.stage = RHI::BarrierStage::DepthStencil;
						newState.layout = RHI::ImageLayout::DepthStencilWrite;
					}
					else
					{
						newState.access = RHI::BarrierAccess::RenderTarget;
						newState.stage = RHI::BarrierStage::RenderTarget;
						newState.layout = RHI::ImageLayout::RenderTarget;
					}
				}

				barrierType = RHI::BarrierType::Image;
			}
			else if (resourceType == ResourceType::Buffer || resourceType == ResourceType::UniformBuffer)
			{
				newState.access = RHI::BarrierAccess::ShaderWrite;
				newState.stage = RHI::BarrierStage::ComputeShader | RHI::BarrierStage::PixelShader;
				barrierType = RHI::BarrierType::Buffer;
			}

			auto& newBarrierInfo = m_resourceBarriers.at(currentPass->index).emplace_back();
			newBarrierInfo.passIndex = currentPass->index;
			newBarrierInfo.hasInitialState = false;
			newBarrierInfo.resourceHandle = access.handle;
			newBarrierInfo.accessInfo.type = barrierType;

			if (barrierType == RHI::BarrierType::Image)
			{
				newBarrierInfo.accessInfo.imageBarrier().dstAccess = newState.access;
				newBarrierInfo.accessInfo.imageBarrier().dstStage = newState.stage;
				newBarrierInfo.accessInfo.imageBarrier().dstLayout = newState.layout;
			}
			else if (barrierType == RHI::BarrierType::Buffer)
			{
				newBarrierInfo.accessInfo.bufferBarrier().dstAccess = newState.access;
				newBarrierInfo.accessInfo.bufferBarrier().dstStage = newState.stage;
			}
		};

		auto writeResourceFunc = [&](Ref<RenderGraphPassNodeBase> currentPass, const RenderGraphPassResourceAccess& access) -> void
		{
			const auto& resourceNode = m_resourceNodes.at(access.handle.Get());
			const auto resourceType = resourceNode->GetResourceType();

			bool hasInitialState = false;
			RHI::ResourceState initialState{};
			initialState.access = RHI::BarrierAccess::None;
			initialState.stage = RHI::BarrierStage::None;
			initialState.layout = RHI::ImageLayout::Undefined;

			// If the resource is external (already exists) and has not yet been used, 
			// we will setup the resources state directly
			if (resourceNode->isExternal && resourceUsages.at(access.handle.Get()).empty())
			{
				if (resourceType == ResourceType::Image2D)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetImage2DRaw(Utility::UpcastHandle<RenderGraphImage2DHandle>(access.handle)));
				}
				else if (resourceType == ResourceType::Buffer)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetBufferRaw(Utility::UpcastHandle<RenderGraphBufferHandle>(access.handle)));
				}
				else if (resourceType == ResourceType::UniformBuffer)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetUniformBufferRaw(Utility::UpcastHandle<RenderGraphUniformBufferHandle>(access.handle)));
				}
				else if (resourceType == ResourceType::Image3D)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetImage3DRaw(Utility::UpcastHandle<RenderGraphImage3DHandle>(access.handle)));
				}

				hasInitialState = true;
			}

			if (!resourceUsages.at(access.handle.Get()).empty())
			{
				initialState = resourceUsages.at(access.handle.Get()).back();
				hasInitialState = true;
			}

			RHI::ResourceState newState{};
			RHI::BarrierType barrierType = RHI::BarrierType::None;

			if (access.forcedState != RenderGraphResourceState::None)
			{
				Utility::SetupForcedState(access.forcedState, resourceNode, newState);

				if (resourceType == ResourceType::Image2D || resourceType == ResourceType::Image3D)
				{
					barrierType = RHI::BarrierType::Image;
				}
				else if (resourceType == ResourceType::Buffer || resourceType == ResourceType::UniformBuffer)
				{
					barrierType = RHI::BarrierType::Buffer;
				}
			}
			else
			{
				if (resourceType == ResourceType::Image2D)
				{
					if (currentPass->isComputePass)
					{
						newState.access = RHI::BarrierAccess::ShaderWrite;
						newState.stage = RHI::BarrierStage::ComputeShader;
						newState.layout = RHI::ImageLayout::ShaderWrite;
					}
					else
					{
						RenderGraphResourceNode<RenderGraphImage2D>& image2DNode = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>();

						if (RHI::Utility::IsDepthFormat(image2DNode.resourceInfo.description.format))
						{
							newState.access = RHI::BarrierAccess::DepthStencilWrite;
							newState.stage = RHI::BarrierStage::DepthStencil;
							newState.layout = RHI::ImageLayout::DepthStencilWrite;
						}
						else
						{
							newState.access = RHI::BarrierAccess::RenderTarget;
							newState.stage = RHI::BarrierStage::RenderTarget;
							newState.layout = RHI::ImageLayout::RenderTarget;
						}
					}

					barrierType = RHI::BarrierType::Image;
				}
				else if (resourceType == ResourceType::Image3D)
				{
					if (currentPass->isComputePass)
					{
						newState.access = RHI::BarrierAccess::ShaderWrite;
						newState.stage = RHI::BarrierStage::ComputeShader;
						newState.layout = RHI::ImageLayout::ShaderWrite;
					}
					else
					{
						newState.access = RHI::BarrierAccess::RenderTarget;
						newState.stage = RHI::BarrierStage::RenderTarget;
						newState.layout = RHI::ImageLayout::RenderTarget;
					}

					barrierType = RHI::BarrierType::Image;
				}
				else if (resourceType == ResourceType::Buffer || resourceType == ResourceType::UniformBuffer)
				{
					newState.access = RHI::BarrierAccess::ShaderWrite;
					if (currentPass->isComputePass)
					{
						newState.stage = RHI::BarrierStage::ComputeShader;
					}
					else
					{
						newState.stage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader | RHI::BarrierStage::MeshShader | RHI::BarrierStage::AmplificationShader;
					}

					barrierType = RHI::BarrierType::Buffer;
				}
			}

			resourceUsages.at(access.handle.Get()).emplace_back(newState);

			auto& newBarrierInfo = m_resourceBarriers.at(currentPass->index).emplace_back();
			newBarrierInfo.passIndex = currentPass->index;
			newBarrierInfo.hasInitialState = hasInitialState;
			newBarrierInfo.resourceHandle = access.handle;
			newBarrierInfo.accessInfo.type = barrierType;

			if (barrierType == RHI::BarrierType::Image)
			{
				newBarrierInfo.accessInfo.imageBarrier().srcAccess = initialState.access;
				newBarrierInfo.accessInfo.imageBarrier().srcStage = initialState.stage;
				newBarrierInfo.accessInfo.imageBarrier().srcLayout = initialState.layout;
				newBarrierInfo.accessInfo.imageBarrier().dstAccess = newState.access;
				newBarrierInfo.accessInfo.imageBarrier().dstStage = newState.stage;
				newBarrierInfo.accessInfo.imageBarrier().dstLayout = newState.layout;
			}
			else if (barrierType == RHI::BarrierType::Buffer)
			{
				newBarrierInfo.accessInfo.bufferBarrier().srcAccess = initialState.access;
				newBarrierInfo.accessInfo.bufferBarrier().srcStage = initialState.stage;
				newBarrierInfo.accessInfo.bufferBarrier().dstAccess = newState.access;
				newBarrierInfo.accessInfo.bufferBarrier().dstStage = newState.stage;
			}
		};

		auto readResourceFunc = [&](Ref<RenderGraphPassNodeBase> currentPass, const RenderGraphPassResourceAccess& access) -> void
		{
			const auto& resourceNode = m_resourceNodes.at(access.handle.Get());
			const auto resourceType = resourceNode->GetResourceType();

			bool hasInitialState = false;
			RHI::ResourceState initialState{};
			initialState.access = RHI::BarrierAccess::None;
			initialState.stage = RHI::BarrierStage::None;
			initialState.layout = RHI::ImageLayout::Undefined;

			// If the resource is external (already exists) and has not yet been used, 
			// we will setup the resources state directly
			if (resourceNode->isExternal && resourceUsages.at(access.handle.Get()).empty())
			{
				if (resourceType == ResourceType::Image2D)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetImage2DRaw(Utility::UpcastHandle<RenderGraphImage2DHandle>(access.handle)));
				}
				else if (resourceType == ResourceType::Buffer)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetBufferRaw(Utility::UpcastHandle<RenderGraphBufferHandle>(access.handle)));
				}
				else if (resourceType == ResourceType::UniformBuffer)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetUniformBufferRaw(Utility::UpcastHandle<RenderGraphUniformBufferHandle>(access.handle)));
				}
				else if (resourceType == ResourceType::Image3D)
				{
					initialState = RHI::GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(GetImage3DRaw(Utility::UpcastHandle<RenderGraphImage3DHandle>(access.handle)));
				}

				hasInitialState = true;
			}

			if (!resourceUsages.at(access.handle.Get()).empty())
			{
				initialState = resourceUsages.at(access.handle.Get()).back();
				hasInitialState = true;
			}

			RHI::ResourceState newState{};
			RHI::BarrierType barrierType = RHI::BarrierType::None;

			if (access.forcedState != RenderGraphResourceState::None)
			{
				Utility::SetupForcedState(access.forcedState, resourceNode, newState);

				if (resourceType == ResourceType::Image2D || resourceType == ResourceType::Image3D)
				{
					barrierType = RHI::BarrierType::Image;
				}
				else if (resourceType == ResourceType::Buffer || resourceType == ResourceType::UniformBuffer)
				{
					barrierType = RHI::BarrierType::Buffer;
				}
			}
			else
			{
				if (resourceType == ResourceType::Image2D || resourceType == ResourceType::Image3D)
				{
					newState.access = RHI::BarrierAccess::ShaderRead;
					newState.layout = RHI::ImageLayout::ShaderRead;

					if (currentPass->isComputePass)
					{
						newState.stage = RHI::BarrierStage::ComputeShader;
					}
					else
					{
						newState.stage = RHI::BarrierStage::PixelShader | RHI::BarrierStage::VertexShader;
					}

					barrierType = RHI::BarrierType::Image;
				}
				else if (resourceType == ResourceType::Buffer || resourceType == ResourceType::UniformBuffer)
				{
					newState.access = RHI::BarrierAccess::ShaderRead;
					if (currentPass->isComputePass)
					{
						newState.stage = RHI::BarrierStage::ComputeShader;
					}
					else
					{
						newState.stage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader | RHI::BarrierStage::MeshShader | RHI::BarrierStage::AmplificationShader;
					}
					barrierType = RHI::BarrierType::Buffer;
				}
			}

			if (Utility::IsReadAfterRead(initialState, newState))
			{
				return;
			}

			resourceUsages.at(access.handle.Get()).emplace_back(newState);

			auto& newBarrierInfo = m_resourceBarriers.at(currentPass->index).emplace_back();
			newBarrierInfo.passIndex = currentPass->index;
			newBarrierInfo.hasInitialState = hasInitialState;
			newBarrierInfo.resourceHandle = access.handle;
			newBarrierInfo.accessInfo.type = barrierType;

			if (barrierType == RHI::BarrierType::Image)
			{
				newBarrierInfo.accessInfo.imageBarrier().srcAccess = initialState.access;
				newBarrierInfo.accessInfo.imageBarrier().srcStage = initialState.stage;
				newBarrierInfo.accessInfo.imageBarrier().srcLayout = initialState.layout;
				newBarrierInfo.accessInfo.imageBarrier().dstAccess = newState.access;
				newBarrierInfo.accessInfo.imageBarrier().dstStage = newState.stage;
				newBarrierInfo.accessInfo.imageBarrier().dstLayout = newState.layout;
			}
			else if (barrierType == RHI::BarrierType::Buffer)
			{
				newBarrierInfo.accessInfo.bufferBarrier().srcAccess = initialState.access;
				newBarrierInfo.accessInfo.bufferBarrier().srcStage = initialState.stage;
				newBarrierInfo.accessInfo.bufferBarrier().dstAccess = newState.access;
				newBarrierInfo.accessInfo.bufferBarrier().dstStage = newState.stage;
			}
		};

		for (const auto& pass : m_passNodes)
		{
			if (!pass->IsCulled())
			{
				for (const auto& create : pass->resourceCreates)
				{
					createResourceFunc(pass, create);
				}

				for (const auto& write : pass->resourceWrites)
				{
					writeResourceFunc(pass, write);
				}

				for (const auto& read : pass->resourceReads)
				{
					readResourceFunc(pass, read);
				}
			}

			if (m_standaloneBarriers.size() > static_cast<size_t>(pass->index) && !m_standaloneBarriers.at(pass->index).empty())
			{
				for (auto& barrier : m_standaloneBarriers.at(pass->index))
				{
					// Try set up barrier source info
					if (!resourceUsages.at(barrier.resourceHandle).empty())
					{
						const auto& previousState = resourceUsages.at(barrier.resourceHandle).back();

						if (barrier.accessInfo.type == RHI::BarrierType::Image)
						{
							barrier.accessInfo.imageBarrier().srcAccess = previousState.access;
							barrier.accessInfo.imageBarrier().srcStage = previousState.stage;
							barrier.accessInfo.imageBarrier().srcLayout = previousState.layout;
						}
						else if (barrier.accessInfo.type == RHI::BarrierType::Buffer)
						{
							barrier.accessInfo.bufferBarrier().srcAccess = previousState.access;
							barrier.accessInfo.bufferBarrier().srcStage = previousState.stage;
						}
						
						barrier.hasInitialState = true;
					}

					// Add this barrier as a resource usage.
					auto& newUsage = resourceUsages.at(barrier.resourceHandle).emplace_back();
					if (barrier.accessInfo.type == RHI::BarrierType::Image)
					{
						newUsage.access = barrier.accessInfo.imageBarrier().dstAccess;
						newUsage.stage = barrier.accessInfo.imageBarrier().dstStage;
						newUsage.layout = barrier.accessInfo.imageBarrier().dstLayout;
					}
					else if (barrier.accessInfo.type == RHI::BarrierType::Buffer)
					{
						newUsage.access = barrier.accessInfo.bufferBarrier().dstAccess;
						newUsage.stage = barrier.accessInfo.bufferBarrier().dstStage;
					}
				}
			}
		}
	}

	void RenderGraph::Execute()
	{
		RenderGraphExecutionThread::ExecuteRenderGraph(std::move(*this));
	}

	void RenderGraph::ExecuteImmediate()
	{
		ExecuteInternal(false);
	}

	void RenderGraph::ExecuteImmediateAndWait()
	{
		ExecuteInternal(true);
	}

	RenderGraphImage2DHandle RenderGraph::AddExternalImage2D(RefPtr<RHI::Image2D> image)
	{
		VT_ENSURE(image);

		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(image); registeredHandle != RenderGraphNullHandle())
		{
			return Utility::UpcastHandle<RenderGraphImage2DHandle>(registeredHandle);
		}

		RenderGraphImage2DHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphImage2DHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.description.format = image->GetFormat();

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		RegisterExternalResource(image, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	RenderGraphImage3DHandle RenderGraph::AddExternalImage3D(RefPtr<RHI::Image3D> image)
	{
		VT_ENSURE(image);

		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(image); registeredHandle != RenderGraphNullHandle())
		{
			return Utility::UpcastHandle<RenderGraphImage3DHandle>(registeredHandle);
		}

		RenderGraphImage3DHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphImage3DHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphImage3D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage3D>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;
		node->resourceInfo.description.format = image->GetFormat();

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		RegisterExternalResource(image, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	RenderGraphBufferHandle RenderGraph::AddExternalBuffer(RefPtr<RHI::StorageBuffer> buffer)
	{
		VT_ENSURE(buffer);

		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(buffer); registeredHandle != RenderGraphNullHandle())
		{
			return Utility::UpcastHandle<RenderGraphBufferHandle>(registeredHandle);
		}

		RenderGraphBufferHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphBufferHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, buffer);

		RegisterExternalResource(buffer, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	RenderGraphUniformBufferHandle RenderGraph::AddExternalUniformBuffer(RefPtr<RHI::UniformBuffer> buffer)
	{
		VT_ENSURE(buffer);

		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(buffer); registeredHandle != RenderGraphNullHandle())
		{
			return Utility::UpcastHandle<RenderGraphUniformBufferHandle>(registeredHandle);
		}

		RenderGraphUniformBufferHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphUniformBufferHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, buffer);

		RegisterExternalResource(buffer, INVALID_RESOURCE_HANDLE);

		return resourceHandle;
	}

	void RenderGraph::ExecuteInternal(bool waitForCompletion)
	{
		VT_PROFILE_FUNCTION();

		AllocateConstantsBuffer();
		m_renderContext.SetPerPassConstantsBuffer(m_perPassConstantsBuffer);
		m_renderContext.SetRenderGraphConstantsBuffer(m_renderGraphConstantsBuffer);
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
				Vector<RHI::ResourceBarrierInfo> barrierInfos{};

				for (const auto& transition : m_resourceBarriers.at(passNode->index))
				{
					auto& barrier = barrierInfos.emplace_back(transition.accessInfo);

					if (barrier.type == RHI::BarrierType::Image)
					{
						barrier.imageBarrier().resource = GetResourceRaw(transition.resourceHandle);

						if (!transition.hasInitialState)
						{
							RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), barrier.imageBarrier().resource);
						}
					}
					else if (barrier.type == RHI::BarrierType::Buffer)
					{
						auto resource = GetResourceRaw(transition.resourceHandle);
						Utility::SetupBufferBarrier(barrier.bufferBarrier(), resource);

						if (!transition.hasInitialState)
						{
							RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), resource);
						}
					}
				}

				m_commandBuffer->ResourceBarrier(barrierInfos);
			}

			{
				VT_PROFILE_SCOPE(passNode->name.data());
				m_renderContext.SetCurrentPass(passNode);

				passNode->Execute(*this, m_renderContext);
			}

			m_commandBuffer->EndMarker();

			InsertStandaloneMarkers(passNode->index);

			for (const auto& resourceHandle : m_surrenderableResources.at(passNode->index))
			{
				const auto resource = m_resourceNodes.at(resourceHandle.Get());
				m_transientResourceSystem.SurrenderResource(resourceHandle, resource->hash);
			}
		}

		// Add the barriers specified after last pass
		if (!m_standaloneBarriers.empty())
		{
			Vector<RHI::ResourceBarrierInfo> barrierInfos{};
			barrierInfos.reserve(m_standaloneBarriers.size());

			for (const auto& barriers : m_standaloneBarriers)
			{
				for (const auto& graphBarrier : barriers)
				{
					auto& barrier = barrierInfos.emplace_back(graphBarrier.accessInfo);

					if (graphBarrier.accessInfo.type == RHI::BarrierType::Image)
					{
						barrier.imageBarrier().resource = GetResourceRaw(graphBarrier.resourceHandle);

						if (!graphBarrier.hasInitialState)
						{
							RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), barrier.imageBarrier().resource);
						}
					}
					else if (graphBarrier.accessInfo.type == RHI::BarrierType::Buffer)
					{
						auto resource = GetResourceRaw(graphBarrier.resourceHandle);
						Utility::SetupBufferBarrier(barrier.bufferBarrier(), resource);

						if (!graphBarrier.hasInitialState)
						{
							RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.bufferBarrier(), resource);
						}
					}
				}
			}

			m_commandBuffer->ResourceBarrier(barrierInfos);
		}

		m_renderContext.UploadConstantsData();

		m_commandBuffer->End();

		BindlessResourcesManager::Get().PrepareForRender();

		if (waitForCompletion)
		{
			m_commandBuffer->ExecuteAndWait();
		}
		else
		{
			m_commandBuffer->Execute();
		}


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
		ExtractResources();

		BindlessResourcesManager::Get().UnregisterBuffer(m_perPassConstantsBufferResourceHandle);

		for (const auto& handle : m_registeredBufferResources)
		{
			BindlessResourcesManager::Get().UnregisterBuffer(handle);
		}

		for (const auto& imageViewInfo : m_registeredImageResources)
		{
			BindlessResourcesManager::Get().UnregisterImageView(imageViewInfo.handle, imageViewInfo.viewType);
		}

		for (const auto& alloc : m_temporaryAllocations)
		{
			delete[] alloc;
		}

		m_temporaryAllocations.clear();
	}

	void RenderGraph::AllocateConstantsBuffer()
	{
		// Pass constants
		{
			RenderGraphBufferDesc desc{};
			desc.count = std::max(m_passIndex, 1u);
			desc.elementSize = RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE;
			desc.usage = RHI::BufferUsage::StorageBuffer;
			desc.memoryUsage = RHI::MemoryUsage::CPUToGPU;
			desc.name = "Render Graph Per Pass Constants";

			m_perPassConstantsBuffer = m_transientResourceSystem.AquireBuffer(Utility::GetValueAsHandle<RenderGraphBufferHandle>(m_resourceIndex++), desc);
			m_perPassConstantsBufferResourceHandle = BindlessResourcesManager::Get().RegisterBuffer(m_perPassConstantsBuffer);
		}

		// Render Graph constants
		{
			RenderGraphBufferDesc desc{};
			desc.count = std::max(m_passIndex, 1u);
			desc.elementSize = sizeof(RenderContext::RenderGraphConstants);
			desc.usage = RHI::BufferUsage::UniformBuffer;
			desc.memoryUsage = RHI::MemoryUsage::CPUToGPU;
			desc.name = "Render Graph Constants";

			m_renderGraphConstantsBuffer = m_transientResourceSystem.AquireUniformBuffer(Utility::GetValueAsHandle<RenderGraphUniformBufferHandle>(m_resourceIndex++), desc);
		}
	}

	void RenderGraph::ExtractResources()
	{
		for (const auto& imageExtractionData : m_image2DExtractions)
		{
			if (imageExtractionData.outImagePtr == nullptr)
			{
				continue;
			}

			auto rawImage = GetImage2DRawRef(imageExtractionData.resourceHandle);
			*imageExtractionData.outImagePtr = rawImage;
		}

		for (const auto& bufferExtractionData : m_bufferExtractions)
		{
			if (bufferExtractionData.outBufferPtr == nullptr)
			{
				continue;
			}

			auto rawBuffer = GetBufferRawRef(bufferExtractionData.resourceHandle);
			*bufferExtractionData.outBufferPtr = rawBuffer;
		}
	}

	void RenderGraph::InitializeRuntimeShaderValidator()
	{
#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		m_runtimeShaderValidator.Allocate(*this);
#endif
	}

	void RenderGraph::AddRuntimeShaderValidationBuffers(Builder& builder)
	{
#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		builder.WriteResource(m_runtimeShaderValidator.GetErrorBufferHandle());
#endif
	}

	RenderGraphImage2DHandle RenderGraph::CreateImage2D(const RenderGraphImageDesc& textureDesc)
	{
		VT_ENSURE_MSG(textureDesc.width > 0 && textureDesc.height > 0, "Width and height must not be zero!");

		RenderGraphImage2DHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphImage2DHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphImage2D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage2D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;
		node->isExternal = false;
		node->isGlobal = !m_currentlyInBuilder;
		node->hash = Utility::GetHashFromImageDesc(textureDesc);

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphImage3DHandle RenderGraph::CreateImage3D(const RenderGraphImageDesc& textureDesc)
	{
		VT_ENSURE_MSG(textureDesc.width > 0 && textureDesc.height > 0 && textureDesc.depth > 0, "Width, height and depth must not be zero!");

		RenderGraphImage3DHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphImage3DHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphImage3D>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage3D>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = textureDesc;
		node->isExternal = false;
		node->isGlobal = !m_currentlyInBuilder;
		node->hash = Utility::GetHashFromImageDesc(textureDesc);

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphBufferHandle RenderGraph::CreateBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		VT_ENSURE_MSG(bufferDesc.elementSize > 0 && bufferDesc.count > 0, "Size must not be zero!");
		VT_ENSURE_MSG(EnumValueContainsFlag(bufferDesc.usage, RHI::BufferUsage::StorageBuffer) || EnumValueContainsFlag(bufferDesc.usage, RHI::BufferUsage::IndexBuffer) || EnumValueContainsFlag(bufferDesc.usage, RHI::BufferUsage::VertexBuffer), "Usage flags should contain StorageBuffer, IndexBuffer or VertexBuffer!");

		RenderGraphBufferHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphBufferHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;
		node->isExternal = false;
		node->isGlobal = !m_currentlyInBuilder;
		node->hash = Utility::GetHashFromBufferDesc(bufferDesc);

		node->resourceInfo.description.usage = node->resourceInfo.description.usage;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	RenderGraphUniformBufferHandle RenderGraph::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc)
	{
		VT_ENSURE_MSG(bufferDesc.elementSize > 0 && bufferDesc.count > 0, "Size must not be zero!");
		//VT_ENSURE_MSG(EnumValueContainsFlag(bufferDesc.usage, RHI::BufferUsage::UniformBuffer), "Usage flags should contain UniformBuffer!");
		 
		RenderGraphUniformBufferHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphUniformBufferHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphUniformBuffer>> node = CreateRef<RenderGraphResourceNode<RenderGraphUniformBuffer>>();
		node->handle = resourceHandle;
		node->resourceInfo.description = bufferDesc;
		node->isExternal = false;
		node->isGlobal = !m_currentlyInBuilder;
		node->hash = Utility::GetHashFromBufferDesc(bufferDesc);

		// #TODO_Ivar: We should not append StorageBuffer here!
		node->resourceInfo.description.usage = node->resourceInfo.description.usage | RHI::BufferUsage::StorageBuffer;

		m_resourceNodes.push_back(node);

		return resourceHandle;
	}

	WeakPtr<RHI::ImageView> RenderGraph::GetImage2DView(const RenderGraphImage2DHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		if (!view->IsSwapchainView())
		{
			m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		}

		return view;
	}

	WeakPtr<RHI::Image2D> RenderGraph::GetImage2DRaw(const RenderGraphImage2DHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		// #TODO_Ivar: Move this section to it's own function
		if (!view->IsSwapchainView())
		{
			m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		}

		return image;
	}

	WeakPtr<RHI::Image3D> RenderGraph::GetImage3DRaw(const RenderGraphImage3DHandle resourceHandle)
	{
		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage3D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage3D(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		// #TODO_Ivar: Move this section to it's own function
		m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());

		return image;
	}

	ResourceHandle RenderGraph::GetImage2D(const RenderGraphImage2DHandle resourceHandle, const int32_t mip, const int32_t layer)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetView(mip, layer);

		VT_ENSURE(!view->IsSwapchainView());

		ResourceHandle handle = BindlessResourcesManager::Get().RegisterImageView(view);
		m_registeredImageResources.emplace_back(handle, view->GetViewType());

		return handle;
	}

	ResourceHandle RenderGraph::GetImage2DArray(const RenderGraphImage2DHandle resourceHandle, const int32_t mip)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2D(resourceHandle, imageDesc.description);
		auto view = image->GetArrayView(mip);

		VT_ENSURE(!view->IsSwapchainView());

		ResourceHandle handle = BindlessResourcesManager::Get().RegisterImageView(view);
		m_registeredImageResources.emplace_back(handle, view->GetViewType());

		return handle;
	}

	ResourceHandle RenderGraph::GetImage3D(const RenderGraphImage3DHandle resourceHandle, const int32_t mip, const int32_t layer)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage3D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage3D(resourceHandle, imageDesc.description);
		auto view = image->GetView(mip, layer);

		VT_ENSURE(!view->IsSwapchainView());

		ResourceHandle handle = BindlessResourcesManager::Get().RegisterImageView(view);
		m_registeredImageResources.emplace_back(handle, view->GetViewType());

		return handle;
	}

	ResourceHandle RenderGraph::GetBuffer(const RenderGraphBufferHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = BindlessResourcesManager::Get().RegisterBuffer(buffer);

		m_registeredBufferResources.emplace_back(handle);
		return handle;
	}

	WeakPtr<RHI::StorageBuffer> RenderGraph::GetBufferRaw(const RenderGraphBufferHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(resourceHandle, bufferDesc.description);
		auto handle = BindlessResourcesManager::Get().RegisterBuffer(buffer);

		m_registeredBufferResources.emplace_back(handle);

		return buffer;
	}

	WeakPtr<RHI::StorageBuffer> RenderGraph::GetUniformBufferRaw(const RenderGraphUniformBufferHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBuffer(*reinterpret_cast<const RenderGraphBufferHandle*>(&resourceHandle), bufferDesc.description);
		auto handle = BindlessResourcesManager::Get().RegisterBuffer(buffer);

		m_registeredBufferResources.emplace_back(handle);

		return buffer;
	}

	ResourceHandle RenderGraph::GetUniformBuffer(const RenderGraphUniformBufferHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphUniformBuffer>>().resourceInfo;

		// #TODO_Ivar: Remove cast once we switch to actually using uniform buffers.
		auto buffer = m_transientResourceSystem.AquireBuffer(*reinterpret_cast<const RenderGraphBufferHandle*>(&resourceHandle), bufferDesc.description);
		auto handle = BindlessResourcesManager::Get().RegisterBuffer(buffer);

		m_registeredBufferResources.emplace_back(handle);

		return handle;
	}

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
	ResourceHandle RenderGraph::GetRuntimeShaderValidationErrorBuffer()
	{
		return GetBuffer(m_runtimeShaderValidator.GetErrorBufferHandle());
	}
#endif

	WeakPtr<RHI::RHIResource> RenderGraph::GetResourceRaw(const RenderGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		if (resourceHandle == RenderGraphNullHandle())
		{
			return {};
		}

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);

		WeakPtr<RHI::RHIResource> result{};

		switch (resourceNode->GetResourceType())
		{
			case ResourceType::Image2D:
			{
				const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;
				result = m_transientResourceSystem.AquireImage2D(Utility::UpcastHandle<RenderGraphImage2DHandle>(resourceHandle), imageDesc.description);

				break;
			}

			case ResourceType::Image3D:
			{
				const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage3D>>().resourceInfo;
				result = m_transientResourceSystem.AquireImage3D(Utility::UpcastHandle<RenderGraphImage3DHandle>(resourceHandle), imageDesc.description);

				break;
			}

			case ResourceType::Buffer:
			{
				const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;
				result = m_transientResourceSystem.AquireBuffer(Utility::UpcastHandle<RenderGraphBufferHandle>(resourceHandle), bufferDesc.description);

				break;
			}

			case ResourceType::UniformBuffer:
			{
				const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphUniformBuffer>>().resourceInfo;
				result = m_transientResourceSystem.AquireBuffer(Utility::UpcastHandle<RenderGraphBufferHandle>(resourceHandle), bufferDesc.description); // #TODO_Ivar: Switch once we start using actual uniform buffers

				break;
			}
		}

		return result;
	}

	RefPtr<RHI::Image2D> RenderGraph::GetImage2DRawRef(const RenderGraphImage2DHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage2D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage2DRef(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		// #TODO_Ivar: Move this section to it's own function
		if (!view->IsSwapchainView())
		{
			m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		}

		return image;
	}

	RefPtr<RHI::Image3D> RenderGraph::GetImage3DRawRef(const RenderGraphImage3DHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage3D>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage3DRef(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		return image;
	}

	RefPtr<RHI::StorageBuffer> RenderGraph::GetBufferRawRef(const RenderGraphBufferHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& bufferDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphBuffer>>().resourceInfo;

		auto buffer = m_transientResourceSystem.AquireBufferRef(resourceHandle, bufferDesc.description);
		auto handle = BindlessResourcesManager::Get().RegisterBuffer(buffer);

		m_registeredBufferResources.emplace_back(handle);

		return buffer;
	}

	RefPtr<RHI::StorageBuffer> RenderGraph::GetUniformBufferRawRef(const RenderGraphUniformBufferHandle resourceHandle)
	{
		return RefPtr<RHI::StorageBuffer>();
	}

	RenderGraphResourceHandle RenderGraph::TryGetRegisteredExternalResource(WeakPtr<RHI::RHIResource> resource)
	{
		if (m_registeredExternalResources.contains(resource))
		{
			return m_registeredExternalResources.at(resource);
		}

		return INVALID_RESOURCE_HANDLE;
	}

	void RenderGraph::RegisterExternalResource(WeakPtr<RHI::RHIResource> resource, RenderGraphResourceHandle handle)
	{
		m_registeredExternalResources[resource] = handle;
	}

	void RenderGraph::AddPass(const std::string& name, std::function<void(RenderGraph::Builder&)> createFunc, std::function<void(RenderContext&)>&& executeFunc)
	{
		static_assert(sizeof(executeFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		struct Empty
		{
		};

		Ref<RenderGraphPassNode<Empty>> newNode = CreateRef<RenderGraphPassNode<Empty>>();
		newNode->name = name;
		newNode->index = m_passIndex++;
		newNode->executeFunction = [executeFunc](const Empty&, RenderContext& context)
		{
			executeFunc(context);
		};

		m_passNodes.push_back(newNode);
		m_resourceBarriers.emplace_back();
		m_standaloneMarkers.emplace_back();

		Builder builder{ *this, newNode };
		createFunc(builder);

		AddRuntimeShaderValidationBuffers(builder);
	}

	void RenderGraph::AddMappedBufferUpload(RenderGraphBufferHandle bufferHandle, const void* data, const size_t size, std::string_view name)
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
		tempBuilder.WriteResource(bufferHandle, RenderGraphResourceState::CopyDest);

		AddRuntimeShaderValidationBuffers(tempBuilder);

		newNode->executeFunction = [tempData, size, bufferHandle](const Empty&, RenderContext& context)
		{
			context.MappedBufferUpload(bufferHandle, tempData, size);
		}; 

		m_passNodes.push_back(newNode);
		m_resourceBarriers.emplace_back();
		m_standaloneMarkers.emplace_back(); 

		m_temporaryAllocations.emplace_back(tempData);
	}

	void RenderGraph::AddMappedBufferUpload(RenderGraphUniformBufferHandle bufferHandle, const void* data, const size_t size, std::string_view name)
	{
		// #TODO_Ivar: Replace with correct functionality once uniform buffers are properly implemented.
		AddMappedBufferUpload(*reinterpret_cast<RenderGraphBufferHandle*>(&bufferHandle), data, size, name);
	}

	void RenderGraph::AddStagedBufferUpload(RenderGraphBufferHandle bufferHandle, const void* data, const size_t size, std::string_view name)
	{
		struct Empty
		{
		};

		uint8_t* tempData = new uint8_t[size];
		memcpy_s(tempData, size, data, size);

		Ref<RenderGraphPassNode<Empty>> newNode = CreateRef<RenderGraphPassNode<Empty>>();
		newNode->name = name;
		newNode->index = m_passIndex++;

		RenderGraphBufferDesc stagingDesc{};
		stagingDesc.memoryUsage = RHI::MemoryUsage::CPUToGPU;
		stagingDesc.name = "Staging Buffer";
		stagingDesc.elementSize = size;
		stagingDesc.count = 1;
		stagingDesc.usage = RHI::BufferUsage::TransferSrc | RHI::BufferUsage::StorageBuffer;

		RenderGraphBufferHandle stagingBuffer = CreateBuffer(stagingDesc);
		AddMappedBufferUpload(stagingBuffer, data, size, name);

		Builder tempBuilder{ *this, newNode };
		tempBuilder.WriteResource(bufferHandle, RenderGraphResourceState::CopyDest);
		tempBuilder.ReadResource(stagingBuffer, RenderGraphResourceState::CopySource);

		AddRuntimeShaderValidationBuffers(tempBuilder);

		newNode->executeFunction = [tempData, size, bufferHandle, stagingBuffer](const Empty&, RenderContext& context)
		{
			context.CopyBuffer(stagingBuffer, bufferHandle, size);
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

		newBarrier.hasInitialState = false;
		newBarrier.isPassSpecificUsage = false;
	}

	Ref<GPUReadbackBuffer> RenderGraph::EnqueueBufferReadback(RenderGraphBufferHandle sourceBuffer)
	{
		const auto resourceNode = m_resourceNodes.at(sourceBuffer)->As<RenderGraphResourceNode<RenderGraphBuffer>>();
		const size_t size = resourceNode.resourceInfo.description.count * resourceNode.resourceInfo.description.elementSize;

		Ref<GPUReadbackBuffer> readbackBuffer = CreateRef<GPUReadbackBuffer>(size);
		RenderGraphBufferHandle dstBufferHandle = AddExternalBuffer(readbackBuffer->GetBuffer());

		RefPtr<RHI::Fence> fence = RHI::Fence::Create(RHI::FenceCreateInfo{ false });

		AddPass("Readback Copy Pass", 
		[&](Builder& builder) 
		{
			builder.ReadResource(sourceBuffer, RenderGraphResourceState::CopySource);
			builder.WriteResource(dstBufferHandle, RenderGraphResourceState::CopyDest);
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context)
		{
			context.CopyBuffer(sourceBuffer, dstBufferHandle, size);
			context.Flush(fence);

			JobSystem::SubmitTask([fence, readbackBuffer]() 
			{
				fence->WaitUntilSignaled();
				readbackBuffer->m_isReady = true;
			});
		});

		return readbackBuffer;
	}

	void RenderGraph::EnqueueImage2DExtraction(RenderGraphImage2DHandle resourceHandle, RefPtr<RHI::Image2D>& outImage)
	{
		m_image2DExtractions.emplace_back(resourceHandle, &outImage);
	}

	void RenderGraph::EnqueueBufferExtraction(RenderGraphBufferHandle resourceHandle, RefPtr<RHI::StorageBuffer>& outBuffer)
	{
		m_bufferExtractions.emplace_back(resourceHandle, &outBuffer);
	}

	void RenderGraph::BeginMarker(const std::string& markerName, const glm::vec4& markerColor)
	{
		m_standaloneMarkers[m_passIndex - 1].emplace_back([markerName, markerColor](RefPtr<RHI::CommandBuffer> commandBuffer)
		{
			commandBuffer->BeginMarker(markerName, { markerColor.x, markerColor.y, markerColor.z, markerColor.w });
		});
	}

	void RenderGraph::EndMarker()
	{
		m_standaloneMarkers[m_passIndex - 1].emplace_back([](RefPtr<RHI::CommandBuffer> commandBuffer)
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

	RenderGraphImage2DHandle RenderGraph::Builder::CreateImage2D(const RenderGraphImageDesc& textureDesc, RenderGraphResourceState forceState)
	{
		const auto resourceId = m_renderGraph.CreateImage2D(textureDesc);
		m_pass->resourceCreates.emplace_back(forceState, resourceId);

		return resourceId;
	}

	RenderGraphImage3DHandle RenderGraph::Builder::CreateImage3D(const RenderGraphImageDesc& textureDesc, RenderGraphResourceState forceState)
	{
		const auto resourceId = m_renderGraph.CreateImage3D(textureDesc);
		m_pass->resourceCreates.emplace_back(forceState, resourceId);

		return resourceId;
	}

	RenderGraphBufferHandle RenderGraph::Builder::CreateBuffer(const RenderGraphBufferDesc& bufferDesc, RenderGraphResourceState forceState)
	{
		const auto resourceId = m_renderGraph.CreateBuffer(bufferDesc);
		m_pass->resourceCreates.emplace_back(forceState, resourceId);

		return resourceId;
	}

	RenderGraphUniformBufferHandle RenderGraph::Builder::CreateUniformBuffer(const RenderGraphBufferDesc& bufferDesc, RenderGraphResourceState forceState)
	{
		const auto resourceId = m_renderGraph.CreateUniformBuffer(bufferDesc);
		m_pass->resourceCreates.emplace_back(forceState, resourceId);

		return resourceId;
	}

	RenderGraphImage2DHandle RenderGraph::Builder::AddExternalImage2D(RefPtr<RHI::Image2D> image)
	{
		return m_renderGraph.AddExternalImage2D(image);
	}

	RenderGraphImage3DHandle RenderGraph::Builder::AddExternalImage3D(RefPtr<RHI::Image3D> image)
	{
		return m_renderGraph.AddExternalImage3D(image);
	}

	RenderGraphBufferHandle RenderGraph::Builder::AddExternalBuffer(RefPtr<RHI::StorageBuffer> buffer)
	{
		return m_renderGraph.AddExternalBuffer(buffer);
	}

	RenderGraphUniformBufferHandle RenderGraph::Builder::AddExternalUniformBuffer(RefPtr<RHI::UniformBuffer> buffer)
	{
		return m_renderGraph.AddExternalUniformBuffer(buffer);
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
