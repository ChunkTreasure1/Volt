#include "vtpch.h"
#include "FrameGraph.h"

#include "Volt/Core/Threading/ThreadPool.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/FrameGraph/TransientResourceSystem.h"
#include "Volt/Rendering/FrameGraph/CommandBufferCache.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Image3D.h"

#include "Volt/Utility/ImageUtility.h"

#include <ranges>

namespace Volt
{
	namespace Utility
	{
		inline static void InitializeMemoryBarrier(VkImageMemoryBarrier2& barrier, const FrameGraphResourceAccess& lastAccess, const FrameGraphResourceAccess& currentAccess, VkImageAspectFlags aspectFlags)
		{
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barrier.srcStageMask = lastAccess.dstStage;
			barrier.srcAccessMask = lastAccess.dstAccess;

			barrier.dstStageMask = currentAccess.dstStage;
			barrier.dstAccessMask = currentAccess.dstAccess;

			barrier.oldLayout = lastAccess.dstLayout;
			barrier.newLayout = currentAccess.dstLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.subresourceRange = { aspectFlags, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
		}
	}

	FrameGraph::FrameGraph(TransientResourceSystem& transientResourceSystem, Weak<CommandBuffer> primaryCommandBuffer, CommandBufferCache& commandBufferCache, ThreadPool& threadPool)
		: myTransientResourceSystem(transientResourceSystem), myPrimaryCommandBuffer(primaryCommandBuffer), myCommandBufferCache(commandBufferCache), myThreadPool(threadPool)
	{
	}

	FrameGraph::~FrameGraph()
	{
	}

	void FrameGraph::Compile()
	{
		VT_PROFILE_FUNCTION();

		///// Calculate Ref Count //////
		for (auto& pass : myRenderPassNodes)
		{
			pass->refCount = static_cast<uint32_t>(pass->resourceWrites.size());

			for (const auto& handle : pass->resourceReads)
			{
				myResourceNodes.at(handle).refCount++;
			}

			for (const auto& handle : pass->resourceWrites)
			{
				myResourceNodes.at(handle).producer = pass;
			}
		}

		///// Cull Render Passes /////
		std::vector<FrameGraphResourceNode*> unreferencedResources;
		for (auto& node : myResourceNodes)
		{
			if (node.refCount == 0)
			{
				unreferencedResources.emplace_back(&node);
			}
		}

		while (!unreferencedResources.empty())
		{
			FrameGraphResourceNode* unrefResource = unreferencedResources.back();
			unreferencedResources.pop_back();

			auto producer = unrefResource->producer;
			if (producer.expired() || producer.lock()->hasSideEffect)
			{
				continue;
			}

			auto producerPtr = producer.lock();

			VT_CORE_ASSERT(producerPtr->refCount > 0, "Ref count cannot be zero at this time!");

			producerPtr->refCount--;
			if (producerPtr->refCount == 0)
			{
				for (const auto& handle : producerPtr->resourceReads)
				{
					auto& node = myResourceNodes.at(handle);
					node.refCount--;
					if (node.refCount == 0)
					{
						unreferencedResources.emplace_back(&node);
					}
				}

				producerPtr->isCulled = true;
			}
		}

		///// Find Resource usages /////
		for (auto& pass : myRenderPassNodes)
		{
			if (pass->refCount == 0)
			{
				continue;
			}

			for (const auto& handle : pass->resourceCreates)
			{
				myResourceNodes.at(handle).producer = pass;
			}

			for (const auto& handle : pass->resourceWrites)
			{
				myResourceNodes.at(handle).lastUsage = pass;
			}

			for (const auto& handle : pass->resourceReads)
			{
				myResourceNodes.at(handle).lastUsage = pass;
			}
		}

		///// Create resource barriers /////
		std::unordered_map<FrameGraphResourceHandle, std::vector<FrameGraphResourceAccess>> resourceAccesses;
		myImageBarriers.resize(myRenderPassNodes.size());
		myImageBarrierResources.resize(myRenderPassNodes.size());
		myResourceAccesses.resize(myRenderPassNodes.size());

		for (const auto& renderPass : myRenderPassNodes)
		{
			if (renderPass->IsCulled())
			{
				continue;
			}

			bool passUsesPreviousTexture = false;
			uint32_t lastUsedPass = 0;

			for (int32_t i = (int32_t)renderPass->index - 1; i >= 0; i--)
			{
				const auto& previousRenderPass = myRenderPassNodes.at(i);

				for (const auto& write : renderPass->resourceWrites)
				{
					// Check previous writes
					auto it = std::find(previousRenderPass->resourceWrites.begin(), previousRenderPass->resourceWrites.end(), write);
					if (it != previousRenderPass->resourceWrites.end())
					{
						passUsesPreviousTexture = true;
					}

					break;
				}

				for (const auto& read : renderPass->resourceReads)
				{
					// Check previous reads
					auto it = std::find(previousRenderPass->resourceWrites.begin(), previousRenderPass->resourceWrites.end(), read);
					if (it != previousRenderPass->resourceWrites.end())
					{
						passUsesPreviousTexture = true;
					}

					break;
				}

				if (passUsesPreviousTexture)
				{
					lastUsedPass = i;
					break;
				}
			}

			if (passUsesPreviousTexture)
			{
				const auto& lastRenderPass = myRenderPassNodes.at(renderPass->index - 1);
				auto& barrier = myExecutionBarriers[renderPass->index];

				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				barrier.pNext = nullptr;
				barrier.srcStageMask = lastRenderPass->isComputePass ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.dstStageMask = renderPass->isComputePass ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.srcAccessMask = lastRenderPass->isComputePass ? VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT : VK_ACCESS_2_NONE;
				barrier.dstAccessMask = renderPass->isComputePass ? VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT : VK_ACCESS_2_NONE;
			}

			for (const auto& write : renderPass->resourceWrites)
			{
				const auto& node = myResourceNodes.at(write);
				const bool isDepthResource = Utility::IsDepthFormat(node.resource.specification.format);
				const VkImageAspectFlags imageAspect = isDepthResource ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

				VkPipelineStageFlagBits2 dstStage = 0;
				VkAccessFlagBits2 dstAccessFlags = 0;
				VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				FrameGraphResourceAccess srcAccess{};

				// If this is the first access, use the images layout
				if (resourceAccesses[write].empty())
				{
					srcAccess = { VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,  (!node.resource.image.expired()) ? node.resource.image.lock()->GetLayout() : VK_IMAGE_LAYOUT_UNDEFINED };
				}
				else
				{
					srcAccess = resourceAccesses[write].back();
				}

				if (isDepthResource)
				{
					dstStage = renderPass->isComputePass ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
					dstAccessFlags = renderPass->isComputePass ? VK_ACCESS_2_SHADER_WRITE_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dstLayout = renderPass->isComputePass ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				}
				else
				{
					dstStage = renderPass->isComputePass ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
					dstAccessFlags = renderPass->isComputePass ? VK_ACCESS_2_SHADER_WRITE_BIT : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
					dstLayout = renderPass->isComputePass ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				bool isWriteToWrite = srcAccess.dstAccess == VK_ACCESS_2_SHADER_WRITE_BIT || srcAccess.dstAccess == VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

				if (srcAccess.dstLayout == dstLayout && srcAccess.dstAccess == dstAccessFlags && srcAccess.dstStage == dstStage && !isWriteToWrite)
				{
					continue;
				}

				auto& imageBarrier = myImageBarriers.at(renderPass->index).emplace_back();
				myImageBarrierResources.at(renderPass->index).emplace_back(node.handle);

				FrameGraphResourceAccess dstAccess{ dstStage, dstAccessFlags, dstLayout };
				Utility::InitializeMemoryBarrier(imageBarrier, srcAccess, dstAccess, imageAspect);

				resourceAccesses[write].emplace_back(dstAccess);
				myResourceAccesses[renderPass->index].emplace_back(write, dstAccess);
			}

			for (const auto& read : renderPass->resourceReads)
			{
				const auto& node = myResourceNodes.at(read);
				const bool isDepthResource = Utility::IsDepthFormat(node.resource.specification.format);
				const VkImageAspectFlags imageAspect = isDepthResource ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

				VkPipelineStageFlagBits2 dstStage = 0;
				VkAccessFlagBits2 dstAccessFlags = 0;
				VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				FrameGraphResourceAccess srcAccess{};

				// If this is the first access, use the images layout
				if (resourceAccesses[read].empty())
				{
					srcAccess = { VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,  (!node.resource.image.expired()) ? node.resource.image.lock()->GetLayout() : VK_IMAGE_LAYOUT_UNDEFINED };
				}
				else
				{
					srcAccess = resourceAccesses[read].back();
				}

				if (isDepthResource)
				{
					dstStage = renderPass->isComputePass ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
					dstAccessFlags = VK_ACCESS_2_SHADER_READ_BIT;
					dstLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
				}
				else
				{
					dstStage = renderPass->isComputePass ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
					dstAccessFlags = VK_ACCESS_2_SHADER_READ_BIT;
					dstLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

				if (srcAccess.dstLayout == dstLayout && srcAccess.dstAccess == dstAccessFlags && srcAccess.dstStage == dstStage)
				{
					continue;
				}

				auto& imageBarrier = myImageBarriers.at(renderPass->index).emplace_back();
				myImageBarrierResources.at(renderPass->index).emplace_back(node.handle);

				FrameGraphResourceAccess dstAccess{ dstStage, dstAccessFlags, dstLayout };
				Utility::InitializeMemoryBarrier(imageBarrier, srcAccess, dstAccess, imageAspect);

				resourceAccesses[read].emplace_back(dstAccess);
				myResourceAccesses[renderPass->index].emplace_back(read, dstAccess);
			}
		}
	}

	void FrameGraph::Execute()
	{
		VT_PROFILE_FUNCTION();

		auto commandBuffer = myPrimaryCommandBuffer.lock();
		myCommandBufferCache.Reset();
		
		std::vector<std::future<void>> commandBufferFutures;

		for (const auto& renderPassNode : myRenderPassNodes)
		{
			if (renderPassNode->IsCulled())
			{
				continue;
			}

			Ref<CommandBuffer> secondaryCommandBuffer = myCommandBufferCache.GetOrCreateCommandBuffer(commandBuffer);
			
			auto future = myThreadPool.SubmitTask([&, cmdBuffer = secondaryCommandBuffer]() 
			{
				cmdBuffer->Begin();

				if (!myImageBarriers.at(renderPassNode->index).empty())
				{
					auto& barriers = myImageBarriers.at(renderPassNode->index);

					for (uint32_t i = 0; const auto & resource : myImageBarrierResources.at(renderPassNode->index))
					{
						barriers[i].image = GetImageResource(resource).image.lock()->GetHandle();
						i++;
					}

					const bool hasExecutionBarrier = myExecutionBarriers.contains(renderPassNode->index);

					VkDependencyInfo dependencyInfo{};
					dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
					dependencyInfo.pNext = nullptr;
					dependencyInfo.dependencyFlags = 0;
					dependencyInfo.memoryBarrierCount = hasExecutionBarrier ? 1 : 0;
					dependencyInfo.pMemoryBarriers = hasExecutionBarrier ? &myExecutionBarriers.at(renderPassNode->index) : nullptr;
					dependencyInfo.bufferMemoryBarrierCount = 0;
					dependencyInfo.pBufferMemoryBarriers = nullptr;
					dependencyInfo.imageMemoryBarrierCount = (uint32_t)barriers.size();
					dependencyInfo.pImageMemoryBarriers = barriers.data();

					vkCmdPipelineBarrier2(cmdBuffer->GetCurrentCommandBuffer(), &dependencyInfo);
				}

				{
					VT_PROFILE_SCOPE(("Execute: " + renderPassNode->name).c_str());
					renderPassNode->Execute(*this, cmdBuffer);
				}

				cmdBuffer->End();
			});

			commandBufferFutures.emplace_back(std::move(future));
		}

		for (auto& future : commandBufferFutures)
		{
			future.wait();
		}

		for (const auto& secondaryCmdBuffer : myCommandBufferCache.GetUsedCommandBuffers())
		{
			secondaryCmdBuffer->Submit();
		}

		for (uint32_t i = 0; const auto& resource : myResourceNodes)
		{
			if (resource.resource.isExternal)
			{
				if (resource.lastUsage.expired())
				{
					continue;
				}

				for (const auto& access : myResourceAccesses.at(resource.lastUsage.lock()->index))
				{
					if (access.first == i)
					{
						resource.resource.image.lock()->OverrideLayout(access.second.dstLayout);
						break;
					}
				}
			}
			i++;
		}
	}

	FrameGraphResourceHandle FrameGraph::CreateTexture(const FrameGraphTextureSpecification& specification)
	{
		VT_PROFILE_FUNCTION();

		const FrameGraphResourceHandle handle = myResourceIndex++;
		auto& resourceNode = myResourceNodes.emplace_back();
		resourceNode.resource.specification = specification;
		resourceNode.handle = handle;

		return handle;
	}

	FrameGraphResourceHandle FrameGraph::AddExternalTexture(Ref<Image2D> image, const std::string& name, ClearMode clearMode)
	{
		VT_PROFILE_FUNCTION();

		const FrameGraphResourceHandle handle = myResourceIndex++;
		auto& resourceNode = myResourceNodes.emplace_back();
		resourceNode.resource.specification.clearMode = clearMode;
		resourceNode.resource.specification.name = name;
		resourceNode.resource.specification.format = image->GetSpecification().format;
		resourceNode.resource.image = image;
		resourceNode.resource.isExternal = true;
		resourceNode.handle = handle;

		return handle;
	}

	FrameGraphResourceHandle FrameGraph::AddExternalTexture(Ref<Image3D> image, const std::string& name, ClearMode clearMode)
	{
		VT_PROFILE_FUNCTION();

		const FrameGraphResourceHandle handle = myResourceIndex++;
		auto& resourceNode = myResourceNodes.emplace_back();
		resourceNode.resource.specification.clearMode = clearMode;
		resourceNode.resource.specification.name = name;
		resourceNode.resource.specification.format = image->GetSpecification().format;
		resourceNode.resource.image3D = image;
		resourceNode.resource.isExternal = true;
		resourceNode.handle = handle;

		return handle;
	}

	const FrameGraphTexture& FrameGraph::GetImageResource(const FrameGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		VT_CORE_ASSERT(myResourceNodes.size() > static_cast<size_t>(resourceHandle), "[FrameGraph] ResourceHandle {} is not valid!", resourceHandle);

		if (myResourceNodes.at(resourceHandle).resource.image.expired())
		{
			const auto& specification = myResourceNodes.at(resourceHandle).resource.specification;
			myResourceNodes.at(resourceHandle).resource.image = myTransientResourceSystem.AquireTexture(specification, resourceHandle);
		}

		return myResourceNodes.at(resourceHandle).resource;
	}

	void FrameGraph::AddRenderPass(const std::string& name, std::function<void(Builder&)> createFunc, std::function<void(FrameGraphRenderPassResources&, Ref<CommandBuffer> commandBuffer)>&& execFunc)
	{
		VT_PROFILE_SCOPE(("AddRenderPass: " + name).c_str());

		static_assert(sizeof(execFunc) <= 512 && "Execution function must not be larger than 512 bytes!");
		struct Empty
		{};

		Ref<FrameGraphRenderPassNode<Empty>> newNode = CreateRef<FrameGraphRenderPassNode<Empty>>();

		newNode->executeFunction = [execFunc](const Empty& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			execFunc(resources, commandBuffer);
		};

		newNode->data = {};
		newNode->index = myRenderPassIndex++;
		newNode->name = name;

		myRenderPassNodes.emplace_back(newNode);

		Builder builder{ *this, newNode };
		createFunc(builder);
	}

	FrameGraphRenderingInfo FrameGraph::CreateRenderingInfoFromResources(std::vector<FrameGraphTexture> resources)
	{
		VT_PROFILE_FUNCTION();

		FrameGraphRenderingInfo renderingInfo{};

		for (const auto& resource : resources)
		{
			if (Utility::IsDepthFormat(resource.specification.format))
			{
				renderingInfo.hasDepth = true;

				renderingInfo.depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				renderingInfo.depthAttachmentInfo.imageView = resource.image.lock()->GetView();

				if (Utility::IsStencilFormat(resource.specification.format))
				{
					renderingInfo.depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					renderingInfo.depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				}

				renderingInfo.depthAttachmentInfo.loadOp = Utility::VoltToVulkanLoadOp(resource.specification.clearMode);
				renderingInfo.depthAttachmentInfo.clearValue.depthStencil = { resource.specification.clearColor.x, 0 };
			}
			else
			{
				auto& attInfo = renderingInfo.colorAttachmentInfo.emplace_back();
				attInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				attInfo.imageView = resource.image.lock()->GetView();
				attInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attInfo.loadOp = Utility::VoltToVulkanLoadOp(resource.specification.clearMode);
				attInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attInfo.clearValue = { resource.specification.clearColor.x, resource.specification.clearColor.y, resource.specification.clearColor.z, resource.specification.clearColor.w };
			}
		}

		return renderingInfo;
	}

	FrameGraph::Builder::Builder(FrameGraph& frameGraph, Ref<FrameGraphRenderPassNodeBase> renderPass)
		: myFrameGraph(frameGraph), myRenderPass(renderPass)
	{
	}

	FrameGraphResourceHandle FrameGraph::Builder::CreateTexture(const FrameGraphTextureSpecification& textureSpecification)
	{
		auto handle = myFrameGraph.CreateTexture(textureSpecification);
		myRenderPass.lock()->resourceCreates.emplace_back(handle);
		myRenderPass.lock()->resourceWrites.emplace_back(handle);

		return handle;
	}

	FrameGraphResourceHandle FrameGraph::Builder::AddExternalTexture(Ref<Image2D> image, const std::string& name, ClearMode clearMode)
	{
		auto handle = myFrameGraph.AddExternalTexture(image, name, clearMode);
		return handle;
	}

	FrameGraphResourceHandle FrameGraph::Builder::AddExternalTexture(Ref<Image3D> image, const std::string& name, ClearMode clearMode)
	{
		auto handle = myFrameGraph.AddExternalTexture(image, name, clearMode);
		return handle;
	}

	void FrameGraph::Builder::SetHasSideEffect()
	{
		myRenderPass.lock()->hasSideEffect = true;
	}

	void FrameGraph::Builder::SetIsComputePass()
	{
		myRenderPass.lock()->isComputePass = true;
	}

	void FrameGraph::Builder::ReadResource(FrameGraphResourceHandle handle)
	{
		myRenderPass.lock()->resourceReads.emplace_back(handle);
	}

	void FrameGraph::Builder::WriteResource(FrameGraphResourceHandle handle)
	{
		if (!myRenderPass.lock()->CreatesResource(handle))
		{
			myRenderPass.lock()->resourceWrites.emplace_back(handle);
		}
	}
}
