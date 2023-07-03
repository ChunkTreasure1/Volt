#pragma once

#include "Volt/Core/Base.h"

#include <vulkan/vulkan.h>
#include <shared_mutex>

namespace Volt
{
	enum class QueueTypeVolt : uint32_t
	{
		Graphics = 0,
		Compute,
		Transfer
	};

	struct PhysicalDeviceCapabilities
	{
		uint32_t maxGraphicsQueueCount = 0;
		uint32_t maxComputeQueueCount = 0;
		uint32_t maxTransferQueueCount = 0;
		
		bool supportsRayTracing = false;
		bool supportsTimestamps = false;

		uint64_t maxPushConstantSize = 0;
		uint64_t availableMemory = 0;
		uint64_t minUBOOffsetAlignment = 0;
		uint64_t minSSBOOffsetAlignment = 0;
		float timestampPeriod = 0;

		std::string_view vendorName;
		std::string_view gpuName;
	};

	struct PhysicalDeviceQueueFamilyIndices
	{
		int32_t graphicsFamilyQueueIndex = -1;
		int32_t computeFamilyQueueIndex = -1;
		int32_t transferFamilyQueueIndex = -1;
	};

	struct PhysicalDeviceInfo
	{
		bool useSeperateComputeQueue = false;
		bool useSeperateTransferQueue = false;

		VkInstance instance = nullptr;
	};

	class PhysicalGraphicsDeviceVolt
	{
	public:
		PhysicalGraphicsDeviceVolt(const PhysicalDeviceInfo& info);
		~PhysicalGraphicsDeviceVolt();

		inline const VkPhysicalDevice GetHandle() const { return myPhysicalDevice; }
		inline const PhysicalDeviceCapabilities& GetCapabilities() const { return myCapabilities; }
		inline const PhysicalDeviceQueueFamilyIndices& GetQueueFamilies() const { return myQueueFamilies; }
		inline const std::vector<VkExtensionProperties>& GetAvailiableExtensions() const { return myAvailiableExtensions; }

		static Scope<PhysicalGraphicsDeviceVolt> Create(const PhysicalDeviceInfo& info);

	private:
		void FetchAvailiableExtensions();
		void CheckRayTracingSupport();

		PhysicalDeviceCapabilities myCapabilities;
		PhysicalDeviceQueueFamilyIndices myQueueFamilies;
		std::vector<VkExtensionProperties> myAvailiableExtensions{};

		VkPhysicalDevice myPhysicalDevice = nullptr;
	};

	struct GraphicsDeviceInfo
	{
		VkPhysicalDeviceFeatures2 requestedFeatures{};
		Weak<PhysicalGraphicsDeviceVolt> physicalDevice;

		uint32_t requestedGraphicsQueues = 1;
		uint32_t requestedComputeQueues = 0;
		uint32_t requestedTransferQueues = 0;
	};

	class GraphicsDeviceVolt
	{
	public:
		GraphicsDeviceVolt(const GraphicsDeviceInfo& info);
		~GraphicsDeviceVolt();
		
		VkCommandBuffer GetCommandBuffer(bool beginCommandBuffer);
		VkCommandBuffer CreateSecondaryCommandBuffer();

		void FlushCommandBuffer(VkCommandBuffer cmdBuffer);
		void FlushCommandBuffer(VkCommandBuffer cmdBuffer, VkCommandPool commandPool, QueueTypeVolt queueType);
		void FlushCommandBuffer(const VkSubmitInfo& submitInfo, VkFence fence, QueueTypeVolt queue, uint32_t wantedQueueIndex = 0);
		void FreeCommandBuffer(VkCommandBuffer cmdBuffer);

		VkResult QueuePresent(const VkPresentInfoKHR& presentInfo, QueueTypeVolt queueType, uint32_t wantedQueueIndex = 0);

		VkCommandBuffer GetSingleUseCommandBuffer(bool beginCommandBuffer, QueueTypeVolt queueType = QueueTypeVolt::Graphics);
		void FlushSingleUseCommandBuffer(VkCommandBuffer cmdBuffer);

		void WaitForIdle() const;
		void WaitForIdle(VkQueue queue) const;

		inline VkDevice GetHandle() const { return myDevice; }
		inline VkQueue GetGraphicsQueue(uint32_t index = 0) const { return myDeviceQueues.at(QueueTypeVolt::Graphics).at(index); }
		inline VkQueue GetComputeQueue(uint32_t index = 0) const { return myDeviceQueues.at(QueueTypeVolt::Compute).at(index); }
		inline VkQueue GetTransferQueue(uint32_t index = 0) const { return myDeviceQueues.at(QueueTypeVolt::Transfer).at(index); }

		inline Weak<PhysicalGraphicsDeviceVolt> GetPhysicalDevice() const { return myPhysicalDevice; }
		static Scope<GraphicsDeviceVolt> Create(const GraphicsDeviceInfo& info);

	private:
		struct SingleUseCommandBufferData
		{
			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;
			VkQueue queue;

			QueueTypeVolt queueType;
		};

		struct PerThreadPoolData
		{
			std::vector<SingleUseCommandBufferData> commandBuffers;
		};

		struct RayTracingFeatures
		{
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeature{};
			VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeature{};
		};

		void EnableRayTracing(std::vector<const char*>& extensionsToEnable);
		const uint32_t GetQueueFamilyIndex(QueueTypeVolt queueType);

		const std::vector<const char*> myValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		VkCommandPool myMainCommandPool = nullptr;
		VkDevice myDevice = nullptr;

		std::unordered_map<std::thread::id, PerThreadPoolData> myPerThreadCommandBuffers; // Thread -> Command Pool & Command Buffers
		
		std::unordered_map<QueueTypeVolt, std::vector<VkQueue>> myDeviceQueues;
		std::unordered_map<QueueTypeVolt, std::mutex> myQueueMutexes;

		std::shared_mutex myCommandBufferDataMutex;

		PhysicalDeviceQueueFamilyIndices myQueueFamilies;

		RayTracingFeatures myRayTracingFeatures;
		Weak<PhysicalGraphicsDeviceVolt> myPhysicalDevice;
	};
}
