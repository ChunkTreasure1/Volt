#pragma once

#include "Volt/Core/UUID.h"

#include "Volt/Rendering/FrameGraph/FrameGraphBlackboard.h"
#include "Volt/Rendering/FrameGraph/FrameGraphRenderPass.h"

#include "Volt/Core/Profiling.h"

namespace Volt
{
	class CommandBuffer;
	class FrameGraphRenderPassResources;
	class FrameGraphResourceRegistry;

	class TransientResourceSystemOld;
	class CommandBufferCache;
	class Image3D;
	class ThreadPool;

	class FrameGraph
	{
	public:
		class Builder
		{
		public:
			Builder(FrameGraph& frameGraph, Ref<FrameGraphRenderPassNodeBase> renderPass);

			FrameGraphResourceHandle CreateTexture(const FrameGraphTextureSpecification& textureSpecification);
			FrameGraphResourceHandle AddExternalTexture(Ref<Image2D> image, const std::string& name, ClearMode clearMode = ClearMode::Clear);
			FrameGraphResourceHandle AddExternalTexture(Ref<Image3D> image, const std::string& name, ClearMode clearMode = ClearMode::Clear);

			void SetHasSideEffect();
			void SetIsComputePass();

			void ReadResource(FrameGraphResourceHandle handle);
			void WriteResource(FrameGraphResourceHandle handle);

		private:
			FrameGraph& myFrameGraph;
			Weak<FrameGraphRenderPassNodeBase> myRenderPass;
		};

		FrameGraph(TransientResourceSystemOld& transientResourceSystem, Weak<CommandBuffer> primaryCommandBuffer, CommandBufferCache& commandBufferCache, ThreadPool& threadPool);
		~FrameGraph();

		void Compile();
		void Execute();

		FrameGraphResourceHandle CreateTexture(const FrameGraphTextureSpecification& specification);
		FrameGraphResourceHandle AddExternalTexture(Ref<Image2D> image, const std::string& name, ClearMode clearMode = ClearMode::Clear);
		FrameGraphResourceHandle AddExternalTexture(Ref<Image3D> image, const std::string& name, ClearMode clearMode = ClearMode::Clear);

		template<typename T>
		T& AddRenderPass(const std::string& name, std::function<void(Builder&, T&)> createFunc, std::function<void(const T&, FrameGraphRenderPassResources&, Ref<CommandBuffer> commandBuffer)>&& execFunc);

		void AddRenderPass(const std::string& name, std::function<void(Builder&)> createFunc, std::function<void(FrameGraphRenderPassResources&, Ref<CommandBuffer> commandBuffer)>&& execFunc);

		const FrameGraphTexture& GetImageResource(const FrameGraphResourceHandle resourceHandle);

		inline const FrameGraphBlackboard& GetBlackboard() const { return myBlackboard; }
		inline FrameGraphBlackboard& GetBlackboard() { return myBlackboard; }

		static FrameGraphRenderingInfo CreateRenderingInfoFromResources(std::vector<FrameGraphTexture> resources);

	private:
		struct DefaultZeroUInt
		{
			uint32_t value = 0;
		};

		std::vector<Ref<FrameGraphRenderPassNodeBase>> myRenderPassNodes;
		std::vector<FrameGraphResourceNode> myResourceNodes;
		std::vector<std::vector<std::pair<FrameGraphResourceHandle, FrameGraphResourceAccess>>> myResourceAccesses;

		FrameGraphBlackboard myBlackboard;
		TransientResourceSystemOld& myTransientResourceSystem;
		CommandBufferCache& myCommandBufferCache;
		ThreadPool& myThreadPool;
		Weak<CommandBuffer> myPrimaryCommandBuffer;

		uint32_t myRenderPassIndex = 0;
		FrameGraphResourceHandle myResourceIndex = 0;

		std::unordered_map<uint32_t, VkMemoryBarrier2> myExecutionBarriers; // Pass Index -> Barrier
		std::vector<std::vector<VkImageMemoryBarrier2>> myImageBarriers; // Render Pass -> Barriers
		std::vector<std::vector<FrameGraphResourceHandle>> myImageBarrierResources; // Render Pass -> Resource
	};

	template<typename T>
	inline T& FrameGraph::AddRenderPass(const std::string& name, std::function<void(FrameGraph::Builder&, T&)> createFunc, std::function<void(const T&, FrameGraphRenderPassResources&, Ref<CommandBuffer> commandBuffer)>&& execFunc)
	{
		VT_PROFILE_SCOPE(("AddRenderPass: " + name).c_str());

		static_assert(sizeof(execFunc) <= 512 && "Execution function must not be larger than 512 bytes!");

		Ref<FrameGraphRenderPassNode<T>> newNode = CreateRef<FrameGraphRenderPassNode<T>>();

		newNode->executeFunction = execFunc;
		newNode->data = {};
		newNode->index = myRenderPassIndex++;
		newNode->name = name;

		myRenderPassNodes.emplace_back(newNode);

		Builder builder{ *this, newNode };
		createFunc(builder, newNode->data);

		return newNode->data;
	}
}
