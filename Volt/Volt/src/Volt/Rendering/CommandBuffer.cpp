#include "vtpch.h"
#include "CommandBuffer.h"


#include "Volt/Core/Application.h"

namespace Volt
{
	CommandBuffer::CommandBuffer(uint32_t count, bool swapchainTarget)
		: myCount(count), mySwapchainTarget(swapchainTarget)
	{
		Invalidate();
	}

	CommandBuffer::CommandBuffer(uint32_t count, Ref<CommandBuffer> primaryCommandBuffer)
		: myCount(count), myInheritedCommandBuffer(primaryCommandBuffer), myLevel(CommandBufferLevel::Secondary)
	{
		Invalidate();
	}

	CommandBuffer::~CommandBuffer()
	{
		Release();
	}

	void CommandBuffer::Begin()
	{
		myHasEnded = false;

		//auto device = GraphicsContextVolt::GetDevice();
		const uint32_t index = GetCurrentIndex();

		/*if (!mySwapchainTarget)
		{
			if (myLevel == CommandBufferLevel::Primary)
			{
				vkWaitForFences(device->GetHandle(), 1, &mySubmitFences.at(index), VK_TRUE, UINT64_MAX);
			}

			VT_VK_CHECK(vkResetCommandPool(device->GetHandle(), myCommandPools.at(index), 0));
		}*/

		switch (myLevel)
		{
			case CommandBufferLevel::Primary: BeginPrimary(); break;
			case CommandBufferLevel::Secondary: BeginSecondary(); break;
		}

		if (myHasTimestampSupport && myLevel == CommandBufferLevel::Primary)
		{
			vkCmdResetQueryPool(myCommandBuffers.at(index), myTimestampQueryPools.at(index), 0, myTimestampQueryCount);
			vkCmdWriteTimestamp(myCommandBuffers.at(index), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, myTimestampQueryPools.at(index), 0);
			myNextAvailableTimestampQuery = 2;

		}
	}

	void CommandBuffer::End()
	{
		const uint32_t index = GetCurrentIndex();

		if (myHasTimestampSupport && myLevel == CommandBufferLevel::Primary)
		{
			vkCmdWriteTimestamp(myCommandBuffers.at(index), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, myTimestampQueryPools.at(index), 1);

		}

		//VT_VK_CHECK(vkEndCommandBuffer(myCommandBuffers.at(index)));

		myHasEnded = true;

		myLastCommandPool = index;
		myCurrentCommandPool = (myCurrentCommandPool + 1) % myCount;
		myLastAvailableTimestampQuery = myNextAvailableTimestampQuery;
	}

	void CommandBuffer::Submit()
	{
		VT_PROFILE_FUNCTION();

		//auto device = GraphicsContextVolt::GetDevice();

		if (!mySwapchainTarget)
		{
			if (myLevel == CommandBufferLevel::Primary)
			{
				const uint32_t index = myLastCommandPool;
				VkSubmitInfo info{};
				info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				info.commandBufferCount = 1;
				info.pCommandBuffers = &myCommandBuffers.at(index);

				//VT_VK_CHECK(vkResetFences(device->GetHandle(), 1, &mySubmitFences.at(index)));

				//device->FlushCommandBuffer(info, mySubmitFences.at(index), myQueueType);
			}
			else if (myInheritedCommandBuffer)
			{
				const uint32_t index = myInheritedCommandBuffer->GetCurrentIndex();

				auto currentCmdBuffer = myCommandBuffers.at(index);

				// #TODO_Ivar: This is a really bad solution to a really weird problem
				if (!myHasEnded)
				{
					End();
				}

				vkCmdExecuteCommands(myInheritedCommandBuffer->GetCurrentCommandBuffer(), 1, &currentCmdBuffer);
			}
		}

		FetchTimestampResults();

	}

	VkCommandBuffer CommandBuffer::GetCurrentCommandBuffer()
	{
		const uint32_t index = GetCurrentIndex();
		return myCommandBuffers.at(index);
	}

	const uint32_t CommandBuffer::GetCurrentIndex()
	{
		if (myLevel == CommandBufferLevel::Primary)
		{
			//const uint32_t index = mySwapchainTarget ? Application::Get().GetWindow().GetSwapchain().GetCurrentFrame() : myCurrentCommandPool;
			return 0;
		}

		auto inheritedCmdBufferPtr = myInheritedCommandBuffer;
		const uint32_t index = inheritedCmdBufferPtr->GetCurrentIndex();

		return index;
	}

	const uint32_t CommandBuffer::GetLastIndex()
	{
		if (myLevel == CommandBufferLevel::Primary)
		{
			return myLastCommandPool;
		}

		auto inheritedCmdBufferPtr = myInheritedCommandBuffer;
		const uint32_t index = inheritedCmdBufferPtr->GetLastIndex();

		return index;
	}

	const uint32_t CommandBuffer::BeginTimestamp()
	{
		return 0;

		//if (!myHasTimestampSupport)
		//{
		//	return 0;
		//}

		//CommandBuffer* cmdBufferPtr = nullptr;

		//if (myLevel == CommandBufferLevel::Secondary)
		//{
		//	auto inheritedCmdBufferPtr = myInheritedCommandBuffer.lock();
		//	cmdBufferPtr = inheritedCmdBufferPtr.get();
		//}
		//else
		//{
		//	cmdBufferPtr = this;
		//}

		//const uint32_t index = GetCurrentIndex();
		//const uint32_t queryId = cmdBufferPtr->myNextAvailableTimestampQuery;
		//cmdBufferPtr->myNextAvailableTimestampQuery += 2;

		//vkCmdWriteTimestamp(myCommandBuffers.at(index), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, cmdBufferPtr->myTimestampQueryPools.at(index), queryId);
		//return queryId;
	}

	void CommandBuffer::EndTimestamp(uint32_t timestampId)
	{
		return;

		//if (!myHasTimestampSupport)
		//{
		//	return;
		//}

		//CommandBuffer* cmdBufferPtr = nullptr;

		//if (myLevel == CommandBufferLevel::Secondary)
		//{
		//	auto inheritedCmdBufferPtr = myInheritedCommandBuffer;
		//	cmdBufferPtr = inheritedCmdBufferPtr.get();
		//}
		//else
		//{
		//	cmdBufferPtr = this;
		//}

		//const uint32_t index = GetCurrentIndex();
		//vkCmdWriteTimestamp(myCommandBuffers.at(index), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, cmdBufferPtr->myTimestampQueryPools.at(index), timestampId + 1);
	}

	const float CommandBuffer::GetExecutionTime(uint32_t frameIndex, uint32_t queryId) const
	{
		if (queryId == UINT32_MAX || queryId / 2 >= myLastAvailableTimestampQuery / 2)
		{
			return 0.f;
		}

		return myGPUExecutionTimes.at(frameIndex).at(queryId / 2);
	}

	Ref<CommandBuffer> CommandBuffer::Create(uint32_t count, bool swapchainTarget)
	{
		return CreateRef<CommandBuffer>(count, swapchainTarget);
	}

	Ref<CommandBuffer> CommandBuffer::Create(uint32_t count, Ref<CommandBuffer> primaryCommandBuffer)
	{
		return CreateRef<CommandBuffer>(count, primaryCommandBuffer);
	}

	void CommandBuffer::Invalidate()
	{
		//auto device = GraphicsContextVolt::GetDevice();

		//if (!mySwapchainTarget)
		//{
		//	myCommandPools.resize(myCount);
		//	myCommandBuffers.resize(myCount);
		//	mySubmitFences.resize(myCount);

		//	// Create Command Pools
		//	for (uint32_t i = 0; i < myCount; i++)
		//	{
		//		VkCommandPoolCreateInfo poolInfo{};
		//		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		//		uint32_t queueFamilyIndex = 0;

		//		switch (myQueueType)
		//		{
		//			case QueueTypeVolt::Graphics:
		//				queueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().graphicsFamilyQueueIndex;
		//				break;
		//			case QueueTypeVolt::Compute:
		//				queueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().computeFamilyQueueIndex;
		//				break;
		//			case QueueTypeVolt::Transfer:
		//				queueFamilyIndex = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies().transferFamilyQueueIndex;
		//				break;
		//			default:
		//				VT_CORE_ASSERT(false, "Invalid QueueType!");
		//				break;
		//		}

		//		poolInfo.queueFamilyIndex = queueFamilyIndex;
		//		poolInfo.flags = 0;

		//		VT_VK_CHECK(vkCreateCommandPool(device->GetHandle(), &poolInfo, nullptr, &myCommandPools.at(i)));
		//	}

		//	// Create Command Buffers
		//	for (uint32_t i = 0; i < myCount; i++)
		//	{
		//		VkCommandBufferAllocateInfo allocInfo{};
		//		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		//		allocInfo.commandPool = myCommandPools.at(i);
		//		allocInfo.level = myLevel == CommandBufferLevel::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		//		allocInfo.commandBufferCount = 1;

		//		VT_VK_CHECK(vkAllocateCommandBuffers(device->GetHandle(), &allocInfo, &myCommandBuffers.at(i)));
		//	}

		//	// Create fences
		//	for (uint32_t i = 0; i < myCount; i++)
		//	{
		//		VkFenceCreateInfo info{};
		//		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//		info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		//		VT_VK_CHECK(vkCreateFence(device->GetHandle(), &info, nullptr, &mySubmitFences.at(i)));
		//	}
		//}
		//else
		//{
		//	const auto& swapchain = Application::Get().GetWindow().GetSwapchain();
		//	myCount = swapchain.GetMaxFramesInFlight();
		//	myCommandPools.resize(myCount);
		//	myCommandBuffers.resize(myCount);

		//	for (uint32_t i = 0; i < myCount; i++)
		//	{
		//		myCommandBuffers[i] = swapchain.GetCommandBuffer(i);
		//		myCommandPools[i] = swapchain.GetCommandPool(i);
		//	}
		//}

		//myHasTimestampSupport = GraphicsContextVolt::GetPhysicalDevice()->GetCapabilities().supportsTimestamps;
		//if (myHasTimestampSupport)
		//{
		//	CreateQueryPools();
		//}
	}

	void CommandBuffer::CreateQueryPools()
	{
		//auto device = GraphicsContextVolt::GetDevice();

		//myTimestampQueryCount = 2 + 2 * MAX_QUERIES;

		//VkQueryPoolCreateInfo info{};
		//info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		//info.pNext = nullptr;
		//info.queryType = VK_QUERY_TYPE_TIMESTAMP;
		//info.queryCount = myTimestampQueryCount;

		//myTimestampQueryPools.resize(myCount);
		//for (auto& pool : myTimestampQueryPools)
		//{
		//	VT_VK_CHECK(vkCreateQueryPool(device->GetHandle(), &info, nullptr, &pool));
		//}

		//myTimestampQueryResults.resize(myCount);
		//for (auto& results : myTimestampQueryResults)
		//{
		//	results.resize(myTimestampQueryCount);
		//}

		//myGPUExecutionTimes.resize(myCount);
		//for (auto& execTimes : myGPUExecutionTimes)
		//{
		//	execTimes.resize(myTimestampQueryCount / 2);
		//}

		/////// Statistics
		//if (myQueueType == QueueTypeVolt::Graphics)
		//{
		//	info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;

		//	myPipelineQueryCount = 7;
		//	info.queryCount = myPipelineQueryCount;
		//	info.pipelineStatistics =
		//		VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
		//		VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
		//		VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
		//		VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
		//		VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
		//		VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
		//		VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

		//	myPipelineStatisticsQueryPools.resize(myCount);
		//	myPipelineStatisticsResults.resize(myCount);
		//	for (auto& pool : myPipelineStatisticsQueryPools)
		//	{
		//		VT_VK_CHECK(vkCreateQueryPool(device->GetHandle(), &info, nullptr, &pool));
		//	}
		//}

	}

	void CommandBuffer::FetchTimestampResults()
	{
		//if (!myHasTimestampSupport || myLastAvailableTimestampQuery == 0)
		//{
		//	return;
		//}

		//auto device = GraphicsContextVolt::GetDevice();
		//const uint32_t index = mySwapchainTarget ? Application::Get().GetWindow().GetSwapchain().GetCurrentFrame() : myLastCommandPool;

		//vkGetQueryPoolResults(device->GetHandle(), myTimestampQueryPools.at(index), 0, myLastAvailableTimestampQuery, myLastAvailableTimestampQuery * sizeof(uint64_t), myTimestampQueryResults.at(index).data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
		//for (uint32_t i = 0; i < myLastAvailableTimestampQuery; i += 2)
		//{
		//	const uint64_t startTime = myTimestampQueryResults.at(index).at(i);
		//	const uint64_t endTime = myTimestampQueryResults.at(index).at(i + 1);

		//	const float nsTime = endTime > startTime ? (endTime - startTime) * GraphicsContextVolt::GetPhysicalDevice()->GetCapabilities().timestampPeriod : 0.f;
		//	myGPUExecutionTimes[index][i / 2] = nsTime * 0.000001f; // Convert to ms
		//}
	}

	void CommandBuffer::FetchPipelineStatistics()
	{
		/*if (!myHasTimestampSupport)
		{
			return;
		}

		auto device = GraphicsContextVolt::GetDevice();
		const uint32_t index = mySwapchainTarget ? Application::Get().GetWindow().GetSwapchain().GetCurrentFrame() : myLastCommandPool;

		if (myQueueType == QueueTypeVolt::Graphics)
		{
			vkGetQueryPoolResults(device->GetHandle(), myPipelineStatisticsQueryPools.at(index), 0, 1, sizeof(RenderPipelineStatistics), &myPipelineStatisticsResults.at(index), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
		}*/
	}

	void CommandBuffer::Release()
	{
		/*auto device = GraphicsContextVolt::GetDevice();
		if (!mySwapchainTarget)
		{
			device->WaitForIdle();

			for (auto& fence : mySubmitFences)
			{
				vkDestroyFence(device->GetHandle(), fence, nullptr);
			}

			mySubmitFences.clear();

			for (auto& pool : myCommandPools)
			{
				vkDestroyCommandPool(device->GetHandle(), pool, nullptr);
			}

			myCommandPools.clear();
			myCommandBuffers.clear();
		}

		for (auto& pool : myTimestampQueryPools)
		{
			vkDestroyQueryPool(device->GetHandle(), pool, nullptr);
		}

		for (auto& pool : myPipelineStatisticsQueryPools)
		{
			vkDestroyQueryPool(device->GetHandle(), pool, nullptr);
		}*/
	}

	void CommandBuffer::BeginPrimary()
	{
		//const uint32_t index = GetCurrentIndex();

		//VkCommandBufferBeginInfo beginInfo{};
		//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		//VT_VK_CHECK(vkBeginCommandBuffer(myCommandBuffers.at(index), &beginInfo));
	}

	void CommandBuffer::BeginSecondary()
	{
		//VkCommandBufferInheritanceRenderingInfo inheritRenderingInfo{};
		//inheritRenderingInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
		//inheritRenderingInfo.pNext = nullptr;
		//inheritRenderingInfo.flags = 0;
		//inheritRenderingInfo.viewMask = 0;
		//inheritRenderingInfo.colorAttachmentCount = 0;
		//inheritRenderingInfo.pColorAttachmentFormats = nullptr;
		//inheritRenderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		//inheritRenderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
		//inheritRenderingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		//VkCommandBufferInheritanceInfo inheritInfo{};
		//inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		//inheritInfo.pNext = &inheritRenderingInfo;
		//inheritInfo.renderPass = nullptr;
		//inheritInfo.subpass = 0;
		//inheritInfo.framebuffer = nullptr;
		//inheritInfo.occlusionQueryEnable = VK_FALSE;
		//inheritInfo.queryFlags = 0;
		//inheritInfo.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
		//	VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
		//	VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
		//	VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
		//	VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
		//	VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
		//	VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

		//const uint32_t index = GetCurrentIndex();

		//VkCommandBufferBeginInfo beginInfo{};
		//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		//beginInfo.pInheritanceInfo = &inheritInfo;

		//VT_VK_CHECK(vkBeginCommandBuffer(myCommandBuffers.at(index), &beginInfo));
	}
}
