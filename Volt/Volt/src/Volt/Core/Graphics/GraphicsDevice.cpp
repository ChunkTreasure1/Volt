#include "vtpch.h"
#include "GraphicsDevice.h"

#include <optick.h>

namespace Volt
{
	namespace Utility
	{
		static const char* VulkanVendorIDToString(uint32_t vendorID)
		{
			switch (vendorID)
			{
				case 0x10DE: return "NVIDIA";
				case 0x1002: return "AMD";
				case 0x8086: return "INTEL";
				case 0x13B5: return "ARM";
			}
			return "Unknown";
		}
	}

	PhysicalGraphicsDevice::PhysicalGraphicsDevice(const PhysicalDeviceInfo& info)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(info.instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPU with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(info.instance, &deviceCount, devices.data());

		VkPhysicalDeviceProperties deviceProperties{};

		for (const auto& device : devices)
		{
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				myPhysicalDevice = device; // Select first discrete GPU. Might not be the optimal one
				break;

			}
		}

		if (!myPhysicalDevice)
		{
			throw std::runtime_error("Failed to find a supported device!");
		}

		// Get properties
		myCapabilities.gpuName = deviceProperties.deviceName;
		myCapabilities.vendorName = Utility::VulkanVendorIDToString(deviceProperties.vendorID);
		myCapabilities.minUBOOffsetAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
		myCapabilities.minSSBOOffsetAlignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
		myCapabilities.supportsTimestamps = deviceProperties.limits.timestampComputeAndGraphics;
		myCapabilities.timestampPeriod = deviceProperties.limits.timestampPeriod;
		myCapabilities.maxPushConstantSize = deviceProperties.limits.maxPushConstantsSize;

		VT_CORE_INFO("[PhysicalDevice] Selecting GPU {0}", myCapabilities.gpuName);

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyCount };
		vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		for (int32_t i = 0; const auto & queueFamily : queueFamilyProperties)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && myQueueFamilies.graphicsFamilyQueueIndex == -1)
			{
				myCapabilities.maxGraphicsQueueCount = queueFamily.queueCount;
				myQueueFamilies.graphicsFamilyQueueIndex = i;
				i++;
				continue;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && myQueueFamilies.computeFamilyQueueIndex == -1)
			{
				myCapabilities.maxComputeQueueCount = queueFamily.queueCount;
				myQueueFamilies.computeFamilyQueueIndex = i;
				i++;
				continue;
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && myQueueFamilies.transferFamilyQueueIndex == -1)
			{
				myCapabilities.maxTransferQueueCount = queueFamily.queueCount;
				myQueueFamilies.transferFamilyQueueIndex = i;
				i++;
				continue;
			}

			if (myQueueFamilies.computeFamilyQueueIndex != -1 &&
				myQueueFamilies.graphicsFamilyQueueIndex != -1 &&
				myQueueFamilies.transferFamilyQueueIndex != -1)
			{
				break;
			}

			i++;
		}

		// Make sure that each type has a queue
		{
			if (!info.useSeperateComputeQueue || myQueueFamilies.computeFamilyQueueIndex == -1)
			{
				myQueueFamilies.computeFamilyQueueIndex = myQueueFamilies.graphicsFamilyQueueIndex;
			}

			if (!info.useSeperateTransferQueue || myQueueFamilies.transferFamilyQueueIndex == -1)
			{
				myQueueFamilies.transferFamilyQueueIndex = myQueueFamilies.graphicsFamilyQueueIndex;
			}
		}

		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(myPhysicalDevice, &memoryProperties);
		for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			myCapabilities.availableMemory += memoryProperties.memoryHeaps[i].size;
		}

		FetchAvailiableExtensions();
		CheckRayTracingSupport();
	}

	PhysicalGraphicsDevice::~PhysicalGraphicsDevice()
	{
	}

	Scope<PhysicalGraphicsDevice> PhysicalGraphicsDevice::Create(const PhysicalDeviceInfo& info)
	{
		return CreateScope<PhysicalGraphicsDevice>(info);
	}

	void PhysicalGraphicsDevice::FetchAvailiableExtensions()
	{
		uint32_t extensionCount = 0;
		VT_VK_CHECK(vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &extensionCount, nullptr));
		myAvailiableExtensions.resize(extensionCount);

		VT_VK_CHECK(vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &extensionCount, myAvailiableExtensions.data()));
	}

	void PhysicalGraphicsDevice::CheckRayTracingSupport()
	{
		uint32_t foundExtensionCount = 0;
		for (const auto& ext : myAvailiableExtensions)
		{
			if (strcmp(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, ext.extensionName) == 0)
			{
				foundExtensionCount++;
			}

			if (strcmp(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, ext.extensionName) == 0)
			{
				foundExtensionCount++;
			}

			if (strcmp(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, ext.extensionName) == 0)
			{
				foundExtensionCount++;
			}
		}

		myCapabilities.supportsRayTracing = foundExtensionCount == 3;
	}

	GraphicsDevice::GraphicsDevice(const GraphicsDeviceInfo& info)
		: myQueueFamilies(info.physicalDevice->GetQueueFamilies())
	{
		myPhysicalDevice = info.physicalDevice;

		auto physicalDevicePtr = info.physicalDevice;
		const auto& capabilities = physicalDevicePtr->GetCapabilities();

		std::vector<VkDeviceQueueCreateInfo> deviceQueueInfos{};
		std::vector<float> queuePriorities(std::max(info.requestedGraphicsQueues, std::max(info.requestedComputeQueues, info.requestedTransferQueues)), 1.f);

		// Setup queue creation
		{
			if (info.requestedGraphicsQueues > 0)
			{
				const uint32_t queueCount = std::min(info.requestedGraphicsQueues, capabilities.maxGraphicsQueueCount);
				if (info.requestedGraphicsQueues > capabilities.maxGraphicsQueueCount)
				{
					VT_CORE_WARN("[GraphicsDevice]: Requested graphics queue count was {0} but only {1} was availiable!", info.requestedGraphicsQueues, capabilities.maxGraphicsQueueCount);
				}

				VkDeviceQueueCreateInfo& queueInfo = deviceQueueInfos.emplace_back();
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.pQueuePriorities = queuePriorities.data();
				queueInfo.queueCount = queueCount;
				queueInfo.queueFamilyIndex = myQueueFamilies.graphicsFamilyQueueIndex;
			}

			if (info.requestedComputeQueues > 0)
			{
				const uint32_t queueCount = std::min(info.requestedComputeQueues, capabilities.maxComputeQueueCount);
				if (info.requestedComputeQueues > capabilities.maxComputeQueueCount)
				{
					VT_CORE_WARN("[GraphicsDevice]: Requested compute queue count was {0} but only {1} was availiable!", info.requestedComputeQueues, capabilities.maxComputeQueueCount);
				}

				VkDeviceQueueCreateInfo& queueInfo = deviceQueueInfos.emplace_back();
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.pQueuePriorities = queuePriorities.data();
				queueInfo.queueCount = queueCount;
				queueInfo.queueFamilyIndex = myQueueFamilies.computeFamilyQueueIndex;
			}

			if (info.requestedTransferQueues > 0)
			{
				const uint32_t queueCount = std::min(info.requestedTransferQueues, capabilities.maxTransferQueueCount);
				if (info.requestedTransferQueues > capabilities.maxTransferQueueCount)
				{
					VT_CORE_WARN("[GraphicsDevice]: Requested transfer queue count was {0} but only {1} was availiable!", info.requestedTransferQueues, capabilities.maxTransferQueueCount);
				}

				VkDeviceQueueCreateInfo& queueInfo = deviceQueueInfos.emplace_back();
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.pQueuePriorities = queuePriorities.data();
				queueInfo.queueCount = queueCount;
				queueInfo.queueFamilyIndex = myQueueFamilies.transferFamilyQueueIndex;
			}
		}

		// Create device
		{
			VkPhysicalDeviceFeatures2 tempFeatures = info.requestedFeatures;

			VkDeviceCreateInfo deviceInfo{};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.queueCreateInfoCount = (uint32_t)deviceQueueInfos.size();
			deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
			deviceInfo.pEnabledFeatures = nullptr;

			std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef VT_ENABLE_VALIDATION
			deviceInfo.enabledLayerCount = (uint32_t)myValidationLayers.size();
			deviceInfo.ppEnabledLayerNames = myValidationLayers.data();

			enabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
#endif

			if (myPhysicalDevice->GetCapabilities().supportsRayTracing)
			{
				EnableRayTracing(enabledExtensions);
				myRayTracingFeatures.rayTracingPipelineFeature.pNext = &tempFeatures;
				deviceInfo.pNext = &myRayTracingFeatures.accelerationStructureFeature;
			}
			else
			{
				deviceInfo.pNext = &tempFeatures;
			}

			deviceInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
			deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();

			VT_VK_CHECK(vkCreateDevice(physicalDevicePtr->GetHandle(), &deviceInfo, nullptr, &myDevice));
		}

		// Get Queues
		{
			uint32_t startIndex = 0;

			if (info.requestedGraphicsQueues > 0)
			{
				const uint32_t queueCount = std::min(info.requestedGraphicsQueues, capabilities.maxGraphicsQueueCount);
				for (uint32_t i = startIndex; i < queueCount + startIndex; i++)
				{
					VkQueue& newQueue = myDeviceQueues[QueueType::Graphics].emplace_back();
					vkGetDeviceQueue(myDevice, (uint32_t)myQueueFamilies.graphicsFamilyQueueIndex, i, &newQueue);
				}

				startIndex += queueCount;
			}

			if (myQueueFamilies.graphicsFamilyQueueIndex != myQueueFamilies.computeFamilyQueueIndex)
			{
				startIndex = 0;
			}

			if (info.requestedComputeQueues > 0)
			{
				const uint32_t queueCount = std::min(info.requestedComputeQueues, capabilities.maxComputeQueueCount);
				for (uint32_t i = startIndex; i < queueCount + startIndex; i++)
				{
					VkQueue& newQueue = myDeviceQueues[QueueType::Compute].emplace_back();
					vkGetDeviceQueue(myDevice, (uint32_t)myQueueFamilies.computeFamilyQueueIndex, startIndex, &newQueue);
				}

				startIndex += queueCount;
			}
			else
			{
				myDeviceQueues[QueueType::Compute].emplace_back(myDeviceQueues[QueueType::Graphics].front());
			}

			if (myQueueFamilies.computeFamilyQueueIndex != myQueueFamilies.transferFamilyQueueIndex)
			{
				startIndex = 0;
			}

			if (info.requestedTransferQueues > 0)
			{
				const uint32_t queueCount = std::min(info.requestedTransferQueues, capabilities.maxTransferQueueCount);
				for (uint32_t i = startIndex; i < queueCount + startIndex; i++)
				{
					VkQueue& newQueue = myDeviceQueues[QueueType::Transfer].emplace_back();
					vkGetDeviceQueue(myDevice, (uint32_t)myQueueFamilies.transferFamilyQueueIndex, startIndex, &newQueue);
				}

				startIndex += queueCount;
			}
			else
			{
				myDeviceQueues[QueueType::Transfer].emplace_back(myDeviceQueues[QueueType::Graphics].front());
			}

			myQueueMutexes[QueueType::Compute];
			myQueueMutexes[QueueType::Graphics];
			myQueueMutexes[QueueType::Transfer];
		}

		// Create main thread (faster to use) command pool
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = myQueueFamilies.graphicsFamilyQueueIndex;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VT_VK_CHECK(vkCreateCommandPool(myDevice, &poolInfo, nullptr, &myMainCommandPool));
		}
	}

	GraphicsDevice::~GraphicsDevice()
	{
		for (auto& [threadId, poolData] : myPerThreadCommandBuffers)
		{
			for (const auto& data : poolData.commandBuffers)
			{
				vkDestroyCommandPool(myDevice, data.commandPool, nullptr);
			}
		}
		myPerThreadCommandBuffers.clear();

		vkDestroyCommandPool(myDevice, myMainCommandPool, nullptr);
		vkDestroyDevice(myDevice, nullptr);
	}

	Scope<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceInfo& info)
	{
		return CreateScope<GraphicsDevice>(info);
	}

	void GraphicsDevice::EnableRayTracing(std::vector<const char*>& extensionsToEnable)
	{
		extensionsToEnable.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		extensionsToEnable.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		extensionsToEnable.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

		myRayTracingFeatures.rayTracingPipelineFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		myRayTracingFeatures.rayTracingPipelineFeature.rayTracingPipeline = VK_TRUE;

		myRayTracingFeatures.accelerationStructureFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		myRayTracingFeatures.accelerationStructureFeature.accelerationStructure = VK_TRUE;
		myRayTracingFeatures.accelerationStructureFeature.pNext = &myRayTracingFeatures.rayTracingPipelineFeature;
	}

	const uint32_t GraphicsDevice::GetQueueFamilyIndex(QueueType queueType)
	{
		switch (queueType)
		{
			case QueueType::Graphics: return myQueueFamilies.graphicsFamilyQueueIndex;
			case QueueType::Compute: return myQueueFamilies.computeFamilyQueueIndex;
			case QueueType::Transfer: return myQueueFamilies.transferFamilyQueueIndex;
		}

		return 0;
	}

	VkCommandBuffer GraphicsDevice::GetCommandBuffer(bool beginCommandBuffer)
	{
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = myMainCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VT_VK_CHECK(vkAllocateCommandBuffers(myDevice, &allocInfo, &commandBuffer));

		if (beginCommandBuffer)
		{
			VkCommandBufferBeginInfo cmdBufferBegin{};
			cmdBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VT_VK_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBufferBegin));
		}

		return commandBuffer;
	}

	VkCommandBuffer GraphicsDevice::CreateSecondaryCommandBuffer()
	{
		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = myMainCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;

		VT_VK_CHECK(vkAllocateCommandBuffers(myDevice, &allocInfo, &commandBuffer));
		return commandBuffer;
	}

	void GraphicsDevice::FlushCommandBuffer(VkCommandBuffer cmdBuffer)
	{
		FlushCommandBuffer(cmdBuffer, myMainCommandPool, QueueType::Graphics);
	}

	void GraphicsDevice::FlushCommandBuffer(VkCommandBuffer cmdBuffer, VkCommandPool commandPool, QueueType queueType)
	{
		VT_CORE_ASSERT(cmdBuffer != VK_NULL_HANDLE, "Unable to flush null command buffer!");

		VT_VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		VkFence fence;
		VT_VK_CHECK(vkCreateFence(myDevice, &fenceInfo, nullptr, &fence));

		{
			auto& mutex = myQueueMutexes[queueType];
			std::unique_lock<std::mutex> lock{ mutex };
			VT_VK_CHECK(vkQueueSubmit(myDeviceQueues[queueType].front(), 1, &submitInfo, fence));
		}

		VT_VK_CHECK(vkWaitForFences(myDevice, 1, &fence, VK_TRUE, 1000000000));

		vkDestroyFence(myDevice, fence, nullptr);
		vkFreeCommandBuffers(myDevice, commandPool, 1, &cmdBuffer);
	}

	void GraphicsDevice::FlushCommandBuffer(const VkSubmitInfo& submitInfo, VkFence fence, QueueType queue, uint32_t wantedQueueIndex)
	{
		auto& mutex = myQueueMutexes[queue];
		std::unique_lock<std::mutex> lock{ mutex };

		const uint32_t queueIndex = std::clamp(wantedQueueIndex, 0u, uint32_t(myDeviceQueues.at(queue).size()));
		VT_VK_CHECK(vkQueueSubmit(myDeviceQueues.at(queue).at(queueIndex), 1, &submitInfo, fence));
	}

	void GraphicsDevice::FreeCommandBuffer(VkCommandBuffer cmdBuffer)
	{
		vkFreeCommandBuffers(myDevice, myMainCommandPool, 1, &cmdBuffer);
	}

	VkResult GraphicsDevice::QueuePresent(const VkPresentInfoKHR& presentInfo, QueueType queueType, uint32_t wantedQueueIndex)
	{
		auto& mutex = myQueueMutexes[queueType];
		std::unique_lock<std::mutex> lock{ mutex };

		const uint32_t queueIndex = std::clamp(wantedQueueIndex, 0u, uint32_t(myDeviceQueues.at(queueType).size()));
		return vkQueuePresentKHR(myDeviceQueues.at(queueType).at(queueIndex), &presentInfo);
	}

	VkCommandBuffer GraphicsDevice::GetSingleUseCommandBuffer(bool beginCommandBuffer, QueueType queueType)
	{
		if (!myPhysicalDevice)
		{
			return nullptr;
		}

		std::scoped_lock lock{ myCommandBufferDataMutex };

		auto physDevicePtr = myPhysicalDevice;

		const auto threadId = std::this_thread::get_id();

		auto& threadData = myPerThreadCommandBuffers[threadId];
		auto& newCmdBuffer = threadData.commandBuffers.emplace_back();

		// Create pool
		{
			VkCommandPoolCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.queueFamilyIndex = GetQueueFamilyIndex(queueType);
			info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

			VT_VK_CHECK(vkCreateCommandPool(myDevice, &info, nullptr, &newCmdBuffer.commandPool));
		}

		// Create buffer
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandBufferCount = 1;
			allocInfo.commandPool = newCmdBuffer.commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			VT_VK_CHECK(vkAllocateCommandBuffers(myDevice, &allocInfo, &newCmdBuffer.commandBuffer));
		}

		if (beginCommandBuffer)
		{
			VkCommandBufferBeginInfo cmdBufferBegin{};
			cmdBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VT_VK_CHECK(vkBeginCommandBuffer(newCmdBuffer.commandBuffer, &cmdBufferBegin));
		}

		newCmdBuffer.queue = myDeviceQueues[queueType].back();
		newCmdBuffer.queueType = queueType;

		return newCmdBuffer.commandBuffer;
	}

	void GraphicsDevice::FlushSingleUseCommandBuffer(VkCommandBuffer cmdBuffer)
	{
		std::scoped_lock lock{ myCommandBufferDataMutex };

		VT_CORE_ASSERT(cmdBuffer != VK_NULL_HANDLE, "Unable to flush null command buffer!");

		const auto threadId = std::this_thread::get_id();

		VT_CORE_ASSERT(myPerThreadCommandBuffers.find(threadId) != myPerThreadCommandBuffers.end(), "Thread ID not found in map. Was this command buffer created on another thread?");
		auto& perThreadData = myPerThreadCommandBuffers.at(threadId);

		auto it = std::find_if(perThreadData.commandBuffers.begin(), perThreadData.commandBuffers.end(), [&](const auto& data)
		{
			return data.commandBuffer == cmdBuffer;
		});
		VT_CORE_ASSERT(it != perThreadData.commandBuffers.end(), "Command buffer not found! Was it created on this thread?");
		VT_VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		VkFence fence;
		VT_VK_CHECK(vkCreateFence(myDevice, &fenceInfo, nullptr, &fence));

		{
			auto& mutex = myQueueMutexes[it->queueType];
			std::unique_lock<std::mutex> queueLock{ mutex };
			VT_VK_CHECK(vkQueueSubmit(it->queue, 1, &submitInfo, fence));
		}

		VT_VK_CHECK(vkWaitForFences(myDevice, 1, &fence, VK_TRUE, 1000000000));

		vkDestroyFence(myDevice, fence, nullptr);
		vkDestroyCommandPool(myDevice, it->commandPool, nullptr);
		perThreadData.commandBuffers.erase(it);
	}

	void GraphicsDevice::WaitForIdle() const
	{
		vkDeviceWaitIdle(myDevice);
	}

	void GraphicsDevice::WaitForIdle(VkQueue queue) const
	{
		vkQueueWaitIdle(queue);
	}
}
