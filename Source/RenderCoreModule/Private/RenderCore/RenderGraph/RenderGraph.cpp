#include "rcpch.h"
#include "RenderCore/RenderGraph/RenderGraph.h"

#include "RenderCore/RenderGraph/RenderGraphCommon.h"
#include "RenderCore/RenderGraph/RenderGraphExecutionThread.h"
#include "RenderCore/RenderGraph/RenderGraphPass.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "RenderCore/RenderGraph/GPUReadbackBuffer.h"
#include "RenderCore/RenderGraph/GPUReadbackImage.h"
#include "RenderCore/RenderGraph/RenderContext.h"

#include "RenderCore/Resources/BindlessResourcesManager.h"

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Images/ImageUtility.h>
#include <RHIModule/Images/ImageView.h>
#include <RHIModule/Utility/ResourceUtility.h>
#include <RHIModule/Synchronization/Fence.h>

#include <JobSystem/JobSystem.h>

#include <CoreUtilities/EnumUtils.h>
#include <CoreUtilities/Profiling/Profiling.h>
#include <CoreUtilities/ComparisonHelpers.h>

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
					RenderGraphResourceNode<RenderGraphImage>& image2DNode = resource->As<RenderGraphResourceNode<RenderGraphImage>>();
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
		: m_commandBuffer(commandBuffer)
	{
		InitializeRuntimeShaderValidator();
	}

	RenderGraph::~RenderGraph()
	{
	}

	inline RHI::ResourceState GetWriteStateForRasterizedImage2D(Ref<RenderGraphResourceNodeBase> resourceNode)
	{
		VT_ENSURE(resourceNode->GetResourceType() == ResourceType::Image2D);

		RenderGraphResourceNode<RenderGraphImage>& image2DNode = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>();
		RHI::ResourceState resultState;

		if (RHI::Utility::IsDepthFormat(image2DNode.resourceInfo.description.format))
		{
			resultState.access = RHI::BarrierAccess::DepthStencilWrite | RHI::BarrierAccess::DepthStencilRead;
			resultState.stage = RHI::BarrierStage::DepthStencil;
			resultState.layout = RHI::ImageLayout::DepthStencilWrite;
		}
		else
		{
			resultState.access = RHI::BarrierAccess::RenderTarget;
			resultState.stage = RHI::BarrierStage::RenderTarget;
			resultState.layout = RHI::ImageLayout::RenderTarget;
		}

		return resultState;
	}

	inline bool GetIsWriteFromAccessMask(RHI::BarrierAccess access)
	{
		if (EnumValueContainsFlag(access, RHI::BarrierAccess::ShaderWrite) ||
			EnumValueContainsFlag(access, RHI::BarrierAccess::DepthStencilWrite) ||
			EnumValueContainsFlag(access, RHI::BarrierAccess::CopyDest) ||
			EnumValueContainsFlag(access, RHI::BarrierAccess::VideoEncodeWrite) ||
			EnumValueContainsFlag(access, RHI::BarrierAccess::VideoDecodeWrite))
		{
			return true;
		}

		return false;
	}

	void RenderGraph::Compile()
	{
		VT_PROFILE_FUNCTION();
		VT_ENSURE_MSG(!m_hasBeenCompiled, "A RenderGraph must not be compiled more than once!");

#ifndef VT_DIST
		// Validate standalone markers
		[[maybe_unused]] size_t markerCount = 0;
		for (const auto& markers : m_standaloneMarkers)
		{
			markerCount += markers.size();
		}

		VT_ENSURE_MSG(markerCount % 2u == 0, "There must be a EndMarker for every BeginMarker!");
#endif

		m_compiledPasses.resize(m_passIndex);

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
			VT_ENSURE_MSG(producer, "Node should always have a producer!");

			if (producer->hasSideEffect)
			{
				continue;
			}

			VT_ENSURE_MSG(producer->refCount > 0, "Ref count cannot be zero at this time!");

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
		for (const auto& resource : m_resourceNodes)
		{
			if (!resource->lastUsage || resource->isExternal)
			{
				continue;
			}

			const uint32_t passIndex = resource->lastUsage->index;
			m_compiledPasses[passIndex].surrenderableResources.emplace_back(resource->handle);
		}
		
		struct ResourceState
		{
			Weak<RenderGraphPassNodeBase> previousUsage;
			RHI::ResourceState currentState;
			bool isWriteState = false;
		};

		struct RGResourceStateTracker
		{
			RGResourceStateTracker(size_t resourceCount) { resourceStates.resize(resourceCount); }

			inline ResourceState& GetState(RenderGraphResourceHandle resourceHandle) { return resourceStates.at(resourceHandle.Get()); }

			Vector<ResourceState> resourceStates;

		} resourceStateTracker(m_resourceNodes.size());

		// Add all the external resources into the resource state tracker.
		for (const auto& resourceNode : m_resourceNodes)
		{
			if (!resourceNode->isExternal)
			{
				continue;
			}

			const auto resourceType = resourceNode->GetResourceType();
			auto resourceTracker = RHI::GraphicsContext::GetResourceStateTracker();

			if (IsEqualToAny(resourceType, ResourceType::Image2D, ResourceType::Image3D))
			{
				resourceStateTracker.GetState(resourceNode->handle).currentState = resourceTracker->GetCurrentResourceState(GetImageRaw(Utility::UpcastHandle<RenderGraphImageHandle>(resourceNode->handle)));
			}
			else if (resourceType == ResourceType::Buffer)
			{
				resourceStateTracker.GetState(resourceNode->handle).currentState = resourceTracker->GetCurrentResourceState(GetBufferRaw(Utility::UpcastHandle<RenderGraphBufferHandle>(resourceNode->handle)));
			}
			else if (resourceType == ResourceType::UniformBuffer)
			{
				resourceStateTracker.GetState(resourceNode->handle).currentState = resourceTracker->GetCurrentResourceState(GetUniformBufferRaw(Utility::UpcastHandle<RenderGraphUniformBufferHandle>(resourceNode->handle)));
			}
		}

		///// Setup Barriers /////
		for (const auto& pass : m_passNodes)
		{
			auto& compiledPass = m_compiledPasses.at(pass->index);
			compiledPass.name = pass->name;

			if (!pass->IsCulled())
			{
				for (const auto& resourceCreate : pass->resourceCreates)
				{
					VT_ENSURE_MSG(resourceCreate.forcedState == RenderGraphResourceState::None, "Forced state is not supported when a resource is created.");

					const auto& resourceNode = m_resourceNodes.at(resourceCreate.handle.Get());
					const auto resourceType = resourceNode->GetResourceType();

					RHI::ResourceState newState{};

					if (pass->isComputePass)
					{
						// For compute shaders, all the resource types have the same accesses
						newState.access = RHI::BarrierAccess::ShaderWrite;
						newState.stage = RHI::BarrierStage::ComputeShader;
						newState.layout = RHI::ImageLayout::ShaderWrite;
					}
					else
					{
						if (resourceType == ResourceType::Image2D)
						{
							newState = GetWriteStateForRasterizedImage2D(resourceNode);
						}
						else if (IsEqualToAny(resourceType, ResourceType::Image3D, ResourceType::Buffer))
						{
							newState.access = RHI::BarrierAccess::ShaderWrite;
							newState.stage = RHI::BarrierStage::PixelShader | RHI::BarrierStage::VertexShader;
							newState.layout = RHI::ImageLayout::ShaderWrite;
						}

						VT_ENSURE(resourceType != ResourceType::UniformBuffer);
					}

					auto& resourceState = resourceStateTracker.GetState(resourceCreate.handle);
					resourceState.currentState = newState;
					resourceState.previousUsage = pass;
					resourceState.isWriteState = true;

					if (IsEqualToAny(resourceType, ResourceType::Image2D, ResourceType::Image3D))
					{
						auto& newBarrier = compiledPass.prePassBarriers.AddBarrier(RHI::BarrierType::Image, resourceCreate.handle);
						newBarrier.imageBarrier().dstAccess = newState.access;
						newBarrier.imageBarrier().dstStage = newState.stage;
						newBarrier.imageBarrier().dstLayout = newState.layout;
					}
					else if (resourceType == ResourceType::Buffer)
					{
						compiledPass.GetGlobalBarrier().dstAccess |= newState.access;
						compiledPass.GetGlobalBarrier().dstStage |= newState.stage;
					}
				}

				for (const auto& resourceWrite : pass->resourceWrites)
				{
					const auto& resourceNode = m_resourceNodes.at(resourceWrite.handle.Get());
					const auto resourceType = resourceNode->GetResourceType();

					// We start by figuring out the state that we want to take the resource to.
					RHI::ResourceState newState{};
					if (resourceWrite.forcedState != RenderGraphResourceState::None)
					{
						Utility::SetupForcedState(resourceWrite.forcedState, resourceNode, newState);
					}
					else
					{
						if (pass->isComputePass)
						{
							// For compute shaders, all the resource types have the same accesses
							newState.access = RHI::BarrierAccess::ShaderWrite;
							newState.stage = RHI::BarrierStage::ComputeShader;
							newState.layout = RHI::ImageLayout::ShaderWrite;
						}
						else
						{
							// For Image2D there are a couple of cases to consider.
							// It could be a color image, or a depth image, and needs
							// to be setup accordingly.
							if (resourceType == ResourceType::Image2D)
							{
								newState = GetWriteStateForRasterizedImage2D(resourceNode);
							}
							else if (IsEqualToAny(resourceType, ResourceType::Image3D, ResourceType::Buffer, ResourceType::UniformBuffer))
							{
								newState.access = RHI::BarrierAccess::ShaderWrite;
								newState.stage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader | RHI::BarrierStage::MeshShader | RHI::BarrierStage::AmplificationShader;
								newState.layout = RHI::ImageLayout::ShaderWrite;
							}
						}
					}

					// Handle cases
					auto& resourceState = resourceStateTracker.GetState(resourceWrite.handle);

					const bool isSameLayoutType = IsEqualToAny(resourceType, ResourceType::Image2D, ResourceType::Image3D) ? newState.layout == resourceState.currentState.layout : true;
					const bool isBufferType = IsEqualToAny(resourceType, ResourceType::Buffer, ResourceType::UniformBuffer);


					// Handle cases 2, 4, 5
					if (isBufferType || isSameLayoutType)
					{
						compiledPass.GetGlobalBarrier().srcAccess |= resourceState.currentState.access;
						compiledPass.GetGlobalBarrier().srcStage |= resourceState.currentState.stage;
						compiledPass.GetGlobalBarrier().dstAccess |= newState.access;
						compiledPass.GetGlobalBarrier().dstStage |= newState.stage;
					}
					// It's not a buffer and the image needs to transition layout, handle case 9.
					else
					{
						auto& newBarrier = compiledPass.prePassBarriers.AddBarrier(RHI::BarrierType::Image, resourceWrite.handle);
						newBarrier.imageBarrier().srcAccess = resourceState.currentState.access;
						newBarrier.imageBarrier().srcStage = resourceState.currentState.stage;
						newBarrier.imageBarrier().srcLayout = resourceState.currentState.layout;
						newBarrier.imageBarrier().dstAccess = newState.access;
						newBarrier.imageBarrier().dstStage = newState.stage;
						newBarrier.imageBarrier().dstLayout = newState.layout;
					}

					resourceState.currentState = newState;
					resourceState.isWriteState = true;
					resourceState.previousUsage = pass;
				}

				for (const auto& resourceRead : pass->resourceReads)
				{
					const auto& resourceNode = m_resourceNodes.at(resourceRead.handle.Get());
					const auto resourceType = resourceNode->GetResourceType();

					// We start by figuring out the state that we want to take the resource to.
					RHI::ResourceState newState{};
					if (resourceRead.forcedState != RenderGraphResourceState::None)
					{
						Utility::SetupForcedState(resourceRead.forcedState, resourceNode, newState);
					}
					else
					{
						// When the resource is being read, the access and layout is the same
						// for both compute and rasterization passes.
						newState.access = RHI::BarrierAccess::ShaderRead;
						newState.layout = RHI::ImageLayout::ShaderRead;

						if (pass->isComputePass)
						{
							newState.stage = RHI::BarrierStage::ComputeShader;
						}
						else
						{
							newState.stage = RHI::BarrierStage::VertexShader | RHI::BarrierStage::PixelShader | RHI::BarrierStage::MeshShader | RHI::BarrierStage::AmplificationShader;
						}
					}

					// Handle cases
					auto& resourceState = resourceStateTracker.GetState(resourceRead.handle);

					// Handle case 1 and 3
					if (!resourceState.isWriteState)
					{
						// As both the previous and the current access are read operations, no barrier is required.
						continue;
					}

					const bool isBufferType = IsEqualToAny(resourceType, ResourceType::Buffer, ResourceType::UniformBuffer);


					// Handle case 8
					if (isBufferType)
					{
						compiledPass.GetGlobalBarrier().srcAccess |= resourceState.currentState.access;
						compiledPass.GetGlobalBarrier().srcStage |= resourceState.currentState.stage;
						compiledPass.GetGlobalBarrier().dstAccess |= newState.access;
						compiledPass.GetGlobalBarrier().dstStage |= newState.stage;
					}
					// If we reach this point, it's an image that needs a layout transition.
					else
					{
						auto& newBarrier = compiledPass.prePassBarriers.AddBarrier(RHI::BarrierType::Image, resourceRead.handle);
						newBarrier.imageBarrier().srcAccess = resourceState.currentState.access;
						newBarrier.imageBarrier().srcStage = resourceState.currentState.stage;
						newBarrier.imageBarrier().srcLayout = resourceState.currentState.layout;
						newBarrier.imageBarrier().dstAccess = newState.access;
						newBarrier.imageBarrier().dstStage = newState.stage;
						newBarrier.imageBarrier().dstLayout = newState.layout;
					}

					resourceState.currentState = newState;
					resourceState.isWriteState = false;
					resourceState.previousUsage = pass;
				}
			}

			// Handle standalone barriers
			if (m_standaloneBarriers.HasPassBarriers(pass->index))
			{
				for (const auto& barrier : m_standaloneBarriers.GetPassBarriers(pass->index))
				{
					auto& resourceState = resourceStateTracker.GetState(barrier.resourceHandle);

					const bool isSameLayoutType = IsEqualToAny(barrier.type, ResourceType::Image2D, ResourceType::Image3D) ? barrier.newState.layout == resourceState.currentState.layout : true;
					const bool isBufferType = IsEqualToAny(barrier.type, ResourceType::Buffer, ResourceType::UniformBuffer);

					if (isBufferType || isSameLayoutType)
					{
						compiledPass.GetPostPassGlobalBarrier().srcAccess |= resourceState.currentState.access;
						compiledPass.GetPostPassGlobalBarrier().srcStage |= resourceState.currentState.stage;
						compiledPass.GetPostPassGlobalBarrier().dstAccess |= barrier.newState.access;
						compiledPass.GetPostPassGlobalBarrier().dstStage |= barrier.newState.stage;
					}
					// It's not a buffer and the image needs to transition layout, handle case 9.
					else
					{
						auto& newBarrier = compiledPass.postPassBarriers.AddBarrier(RHI::BarrierType::Image, barrier.resourceHandle);
						newBarrier.imageBarrier().srcAccess = resourceState.currentState.access;
						newBarrier.imageBarrier().srcStage = resourceState.currentState.stage;
						newBarrier.imageBarrier().srcLayout = resourceState.currentState.layout;
						newBarrier.imageBarrier().dstAccess = barrier.newState.access;
						newBarrier.imageBarrier().dstStage = barrier.newState.stage;
						newBarrier.imageBarrier().dstLayout = barrier.newState.layout;
					}

					resourceState.currentState = barrier.newState;
					resourceState.isWriteState = GetIsWriteFromAccessMask(barrier.newState.access);
					resourceState.previousUsage = pass;
				}
			}
		}

		m_hasBeenCompiled = true;
	}

	void RenderGraph::Execute()
	{
		RenderGraphExecutionThread::ExecuteRenderGraph(std::move(*this));
	}

	void RenderGraph::ExecuteImmediate()
	{
		ExecuteInternal(true, false);
	}

	void RenderGraph::ExecuteImmediateAndWait()
	{
		ExecuteInternal(true, true);
	}

	RenderGraphImageHandle RenderGraph::AddExternalImage(RefPtr<RHI::Image> image)
	{
		VT_ENSURE(image);

		if (RenderGraphResourceHandle registeredHandle = TryGetRegisteredExternalResource(image); registeredHandle != RenderGraphNullHandle())
		{
			return Utility::UpcastHandle<RenderGraphImageHandle>(registeredHandle);
		}

		RenderGraphImageHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphImageHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphImage>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage>>();
		node->handle = resourceHandle;
		node->isExternal = true;
		node->resourceInfo.isExternal = true;

		node->resourceInfo.description.format = image->GetFormat();
		node->resourceInfo.description.usage = image->GetUsage();
		node->resourceInfo.description.width = image->GetWidth();
		node->resourceInfo.description.height = image->GetHeight();
		node->resourceInfo.description.depth = image->GetDepth();
		node->resourceInfo.description.layers = image->GetLayerCount();
		node->resourceInfo.description.mips = image->GetMipCount();

		if (image->GetType() == RHI::ResourceType::Image2D)
		{
			node->resourceInfo.description.type = ResourceType::Image2D;
		}
		else if (image->GetType() == RHI::ResourceType::Image3D)
		{
			node->resourceInfo.description.type = ResourceType::Image3D;
		}

		m_resourceNodes.push_back(node);
		m_transientResourceSystem.AddExternalResource(resourceHandle, image);

		RegisterExternalResource(image, resourceHandle);

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

		RegisterExternalResource(buffer, resourceHandle);

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

		RegisterExternalResource(buffer, resourceHandle);

		return resourceHandle;
	}

	void RenderGraph::ExecuteInternal(bool waitForCompletedExecution, bool waitForSync)
	{
		VT_PROFILE_FUNCTION();

		struct PassExecutionRange
		{
			uint16_t first;
			uint16_t last;
		};

		constexpr size_t MAX_PASSES_PER_JOB = 20;

		AllocateConstantsBuffer();
		
		m_sharedRenderContext.SetPerPassConstantsBuffer(m_perPassConstantsBuffer);
		m_sharedRenderContext.SetRenderGraphConstantsBuffer(m_renderGraphConstantsBuffer);

		Vector<PassExecutionRange> executionRanges;

		// Setup execution ranges
		{
			const size_t jobCount = m_passNodes.size() / MAX_PASSES_PER_JOB;
			const size_t passRemainder = m_passNodes.size() - jobCount * MAX_PASSES_PER_JOB;

			executionRanges.resize_uninitialized(jobCount + ((passRemainder > 0) ? 1 : 0));

			for (size_t i = 0; i < jobCount; i++)
			{
				const uint16_t rangeOffset = static_cast<uint16_t>(i * MAX_PASSES_PER_JOB);
				executionRanges[i] = { rangeOffset, static_cast<uint16_t>(rangeOffset + MAX_PASSES_PER_JOB) };
			}

			if (passRemainder > 0)
			{
				const uint16_t rangeOffset = static_cast<uint16_t>((executionRanges.size() - 1) * MAX_PASSES_PER_JOB);
				executionRanges.back() = { rangeOffset, static_cast<uint16_t>(rangeOffset + passRemainder) };
			}
		}

		const bool executeLocally = m_passNodes.size() <= MAX_PASSES_PER_JOB;
		const bool allowMultithreadedExecution = false;

		Vector<RefPtr<RHI::CommandBuffer>> secondaryCommandBuffers;
		secondaryCommandBuffers.resize(executionRanges.size());

		auto executeRangeFunc = [this](RefPtr<RHI::CommandBuffer> rangeCmdBuffer, uint16_t first, uint16_t last)
		{
			rangeCmdBuffer->Begin();

			for (uint16_t index = first; index < last; index++)
			{
				const auto& passNode = m_passNodes.at(index);
				auto& compiledPass = m_compiledPasses.at(passNode->index);

				if (passNode->IsCulled())
				{
					//InsertStandaloneMarkersIntoCommandBuffer(passNode->index, rangeCmdBuffer);
					InsertBarriersIntoCommandBuffer(compiledPass.postPassBarriers, rangeCmdBuffer);
					continue;
				}

				rangeCmdBuffer->BeginMarker(passNode->name, { 1.f, 1.f, 1.f, 1.f });

				InsertBarriersIntoCommandBuffer(compiledPass.prePassBarriers, rangeCmdBuffer);

				{
					VT_PROFILE_SCOPE(passNode->name.data());
					RenderContext renderContext(*this, *passNode, m_sharedRenderContext, rangeCmdBuffer);
					passNode->Execute(*this, renderContext);
					renderContext.EndContext();
				}

				InsertBarriersIntoCommandBuffer(compiledPass.postPassBarriers, rangeCmdBuffer);

				rangeCmdBuffer->EndMarker();

				//InsertStandaloneMarkersIntoCommandBuffer(passNode->index, rangeCmdBuffer);

				for (const auto& resourceHandle : compiledPass.surrenderableResources)
				{
					const auto resource = m_resourceNodes.at(resourceHandle.Get());
					m_transientResourceSystem.SurrenderResource(resourceHandle, resource->hash);
				}
			}

			rangeCmdBuffer->End();
		};

		m_sharedRenderContext.BeginContext();

		Vector<std::future<void>> rangeFutures;

		for (uint32_t rangeIndex = 0; const auto& [first, last] : executionRanges)
		{
			// If the pass count only creates one job, we will execute it on this thread instead.
			RefPtr<RHI::CommandBuffer> rangeCmdBuffer = executeLocally ? m_commandBuffer : m_commandBuffer->CreateSecondaryCommandBuffer();
			secondaryCommandBuffers[rangeIndex] = rangeCmdBuffer;

			if (executeLocally || !allowMultithreadedExecution)
			{
				executeRangeFunc(rangeCmdBuffer, first, last);
			}
			else
			{
				rangeFutures.emplace_back(JobSystem::SubmitTask([&executeRangeFunc, rangeCmdBuffer, first, last]()
				{
					executeRangeFunc(rangeCmdBuffer, first, last);
				}));
			}

			rangeIndex++;
		}

		for (auto& future : rangeFutures)
		{
			future.wait();
		}

		m_sharedRenderContext.EndContext();
		BindlessResourcesManager::Get().PrepareForRender();

		// Execute the secondary command buffers generated by the jobs in the main command buffer.
		if (!executeLocally)
		{
			m_commandBuffer->Begin();
			m_commandBuffer->ExecuteSecondaryCommandBuffers(secondaryCommandBuffers);
			m_commandBuffer->End();
		}

		if (waitForSync)
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

	void RenderGraph::InsertStandaloneMarkersIntoCommandBuffer(const uint32_t passIndex, const RefPtr<RHI::CommandBuffer> commandBuffer)
	{
		for (const auto& marker : m_standaloneMarkers.at(passIndex))
		{
			marker(commandBuffer);
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
		for (const auto& imageExtractionData : m_imageExtractions)
		{
			if (imageExtractionData.outImagePtr == nullptr)
			{
				continue;
			}

			auto rawImage = GetImageRawRef(imageExtractionData.resourceHandle);
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

	void RenderGraph::PrintPassBarriers(const Vector<RHI::ResourceBarrierInfo>& barriers)
	{
#ifdef VT_ENABLE_RENDERGRAPH_DEBUG_LOG
		for (const auto& barrier : barriers)
		{
			if (barrier.type == RHI::BarrierType::Image)
			{
			}
			else if (barrier.type == RHI::BarrierType::Buffer)
			{

			}
		}
#endif
	}

	RenderGraphImageHandle RenderGraph::CreateImage(const RenderGraphImageDesc& textureDesc)
	{
		VT_ENSURE_MSG(textureDesc.width > 0 && textureDesc.height > 0 && textureDesc.depth > 0, "Width, height and depth must not be zero!");

		RenderGraphImageHandle resourceHandle = Utility::GetValueAsHandle<RenderGraphImageHandle>(m_resourceIndex++);
		Ref<RenderGraphResourceNode<RenderGraphImage>> node = CreateRef<RenderGraphResourceNode<RenderGraphImage>>();
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

	WeakPtr<RHI::ImageView> RenderGraph::GetImageView(const RenderGraphImageHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		if (!view->IsSwapchainView())
		{
			m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		}

		return view;
	}

	WeakPtr<RHI::Image> RenderGraph::GetImageRaw(const RenderGraphImageHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		// #TODO_Ivar: Move this section to it's own function
		if (!view->IsSwapchainView())
		{
			m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		}

		return image;
	}

	ResourceHandle RenderGraph::GetImage(const RenderGraphImageHandle resourceHandle, const int32_t mip, const int32_t layer)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage(resourceHandle, imageDesc.description);
		auto view = image->GetView(mip, layer);

		VT_ENSURE(!view->IsSwapchainView());

		ResourceHandle handle = BindlessResourcesManager::Get().RegisterImageView(view);
		m_registeredImageResources.emplace_back(handle, view->GetViewType());

		return handle;
	}

	ResourceHandle RenderGraph::GetImageArray(const RenderGraphImageHandle resourceHandle, const int32_t mip)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle.Get());
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImage(resourceHandle, imageDesc.description);
		auto view = image->GetArrayView(mip);

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
			case ResourceType::Image3D:
			{
				const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>().resourceInfo;
				result = m_transientResourceSystem.AquireImage(Utility::UpcastHandle<RenderGraphImageHandle>(resourceHandle), imageDesc.description);

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

	RefPtr<RHI::Image> RenderGraph::GetImageRawRef(const RenderGraphImageHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		const auto& imageDesc = resourceNode->As<RenderGraphResourceNode<RenderGraphImage>>().resourceInfo;

		auto image = m_transientResourceSystem.AquireImageRef(resourceHandle, imageDesc.description);
		auto view = image->GetView();

		// #TODO_Ivar: Move this section to it's own function
		if (!view->IsSwapchainView())
		{
			m_registeredImageResources.emplace_back(BindlessResourcesManager::Get().RegisterImageView(view), view->GetViewType());
		}

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

		return RenderGraphNullHandle{};
	}

	void RenderGraph::RegisterExternalResource(WeakPtr<RHI::RHIResource> resource, RenderGraphResourceHandle handle)
	{
		m_registeredExternalResources[resource] = handle;
	}

	void RenderGraph::InsertBarriersIntoCommandBuffer(const CompiledRenderGraphPass::PassBarriers& passBarriers, const RefPtr<RHI::CommandBuffer>& commandBuffer)
	{
		if (passBarriers.Empty())
		{
			return;
		}

		Vector<RHI::ResourceBarrierInfo> resultBarriers;
		resultBarriers.reserve(passBarriers.GetBarrierCount());

		for (const auto& passBarrier : passBarriers.GetBarriers())
		{
			VT_ENSURE(passBarrier.barrier.type == RHI::BarrierType::Global || passBarrier.resourceHandle != RenderGraphNullHandle{});

			auto& barrier = resultBarriers.emplace_back(passBarrier.barrier);
			if (barrier.type == RHI::BarrierType::Image)
			{
				barrier.imageBarrier().resource = GetResourceRaw(passBarrier.resourceHandle);
			}
			else if (barrier.type == RHI::BarrierType::Buffer)
			{
				barrier.bufferBarrier().resource = GetResourceRaw(passBarrier.resourceHandle);
			}
		}

		commandBuffer->ResourceBarrier(resultBarriers);
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

		newNode->executeFunction = [size, bufferHandle, stagingBuffer](const Empty&, RenderContext& context)
		{
			context.CopyBuffer(stagingBuffer, bufferHandle, size);
		};

		m_passNodes.push_back(newNode);
		m_standaloneMarkers.emplace_back();

		m_temporaryAllocations.emplace_back(tempData);
	}

	void RenderGraph::AddResourceBarrier(RenderGraphResourceHandle resourceHandle, const RenderGraphBarrierInfo& barrierInfo)
	{
		const uint32_t passIndex = m_passNodes.empty() ? 0u : static_cast<uint32_t>(m_passNodes.size() - 1);

		auto& newBarrier = m_standaloneBarriers.AddBarrier(passIndex);

		const auto& resourceNode = m_resourceNodes.at(resourceHandle);
		newBarrier.resourceHandle = resourceHandle;
		newBarrier.newState.access = barrierInfo.dstAccess;
		newBarrier.newState.stage = barrierInfo.dstStage;
		newBarrier.newState.layout = barrierInfo.dstLayout;
		newBarrier.type = resourceNode->GetResourceType();
	}

	Ref<GPUReadbackBuffer> RenderGraph::EnqueueBufferReadback(RenderGraphBufferHandle sourceBuffer)
	{
		const auto& resourceNode = m_resourceNodes.at(sourceBuffer)->As<RenderGraphResourceNode<RenderGraphBuffer>>();
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

	Ref<GPUReadbackImage> RenderGraph::EnqueueImageReadback(RenderGraphImageHandle sourceImage)
	{
		const auto& resourceNode = m_resourceNodes.at(sourceImage)->As<RenderGraphResourceNode<RenderGraphImage>>();

		Ref<GPUReadbackImage> readbackImage = CreateRef<GPUReadbackImage>(resourceNode.resourceInfo.description);
		RenderGraphImageHandle dstImageHandle = AddExternalImage(readbackImage->GetImage());

		RefPtr<RHI::Fence> fence = RHI::Fence::Create(RHI::FenceCreateInfo{ false });

		const uint32_t width = resourceNode.resourceInfo.description.width;
		const uint32_t height = resourceNode.resourceInfo.description.height;
		const uint32_t depth = resourceNode.resourceInfo.description.depth;

		AddPass("Readback Copy Pass",
		[&](Builder& builder)
		{
			builder.ReadResource(sourceImage, RenderGraphResourceState::CopySource);
			builder.WriteResource(dstImageHandle, RenderGraphResourceState::CopyDest);
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context)
		{
			context.CopyImage(sourceImage, dstImageHandle, width, height, depth);
			context.Flush(fence);

			JobSystem::SubmitTask([fence, readbackImage]()
			{
				fence->WaitUntilSignaled();
				readbackImage->m_isReady = true;
			});
		});

		return readbackImage;
	}

	void RenderGraph::EnqueueImageExtraction(RenderGraphImageHandle resourceHandle, RefPtr<RHI::Image>& outImage)
	{
		m_imageExtractions.emplace_back(resourceHandle, &outImage);
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

	RenderGraphImageHandle RenderGraph::Builder::CreateImage(const RenderGraphImageDesc& textureDesc, RenderGraphResourceState forceState)
	{
		const auto resourceId = m_renderGraph.CreateImage(textureDesc);
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

	RenderGraphImageHandle RenderGraph::Builder::AddExternalImage(RefPtr<RHI::Image> image)
	{
		return m_renderGraph.AddExternalImage(image);
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
