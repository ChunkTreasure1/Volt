#pragma once

#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	enum class CommandBufferLevel : uint32_t
	{
		Primary = 0,
		Secondary
	};

	class CommandBuffer
	{
	public:
		CommandBuffer(uint32_t count, bool swapchainTarget);
		CommandBuffer(uint32_t count, Ref<CommandBuffer> primaryCommandBuffer);
		~CommandBuffer();

		void Begin();
		void End();
		void Submit();

		VkCommandBuffer GetCurrentCommandBuffer();
		const uint32_t GetCurrentIndex();
		const uint32_t GetLastIndex();

		const uint32_t BeginTimestamp();
		void EndTimestamp(uint32_t timestampId);

		const float GetExecutionTime(uint32_t frameIndex, uint32_t queryId) const;
		const RenderPipelineStatistics& GetPipelineStatistics(uint32_t frameIndex) const;

		static Ref<CommandBuffer> Create(uint32_t count, bool swapchainTarget = false);
		static Ref<CommandBuffer> Create(uint32_t count, Ref<CommandBuffer> primaryCommandBuffer);

	private:
		inline static constexpr uint32_t MAX_QUERIES = 64;

		void Invalidate();
		void CreateQueryPools();
		void FetchTimestampResults();
		void FetchPipelineStatistics();

		void Release();

		void BeginPrimary();
		void BeginSecondary();

		std::vector<VkCommandPool> myCommandPools;
		std::vector<VkCommandBuffer> myCommandBuffers;
		std::vector<VkFence> mySubmitFences;

		uint32_t myTimestampQueryCount = 0;
		uint32_t myNextAvailableTimestampQuery = 2; // The two first are command buffer total
		uint32_t myLastAvailableTimestampQuery = 0;

		std::vector<VkQueryPool> myTimestampQueryPools;
		std::vector<VkQueryPool> myPipelineStatisticsQueryPools;
		std::vector<std::vector<uint64_t>> myTimestampQueryResults;
		std::vector<std::vector<float>> myGPUExecutionTimes;

		uint32_t myPipelineQueryCount = 0;
		std::vector<RenderPipelineStatistics> myPipelineStatisticsResults;

		CommandBufferLevel myLevel = CommandBufferLevel::Primary;
		Weak<CommandBuffer> myInheritedCommandBuffer;

		bool mySwapchainTarget = false;
		bool myHasTimestampSupport = false;
		bool myHasEnded = false;

		uint32_t myCurrentCommandPool = 0;
		uint32_t myLastCommandPool = 0;
		uint32_t myCount = 0;
	};
}
