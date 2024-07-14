#include "vkpch.h"
#include "VulkanGraphicsDevice.h"

#include "VoltVulkan/Common/VulkanCommon.h"	

#include "VoltVulkan/Graphics/VulkanDeviceQueue.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"

#include "VoltVulkan/Core.h"

#include <vulkan/vulkan.h>
#include <optick.h>

namespace Volt::RHI
{
	struct EnabledFeatures
	{
		// Native
		VkPhysicalDeviceVulkan11Features vulkan11Features;
		VkPhysicalDeviceVulkan12Features vulkan12Features;
		VkPhysicalDeviceVulkan13Features vulkan13Features;

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures;

		// Extensions
		VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeaturesEXT;
		VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeaturesEXT;
		VkDeviceDiagnosticsConfigCreateInfoNV aftermathDiagInfo{};
	};

	static EnabledFeatures s_enabledFeatures{};

	namespace Utility
	{
		inline static void GetEnabledFeatures(WeakPtr<VulkanPhysicalGraphicsDevice> physicalDevice)
		{

			s_enabledFeatures.vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
			s_enabledFeatures.vulkan11Features.pNext = nullptr;
			s_enabledFeatures.vulkan11Features.shaderDrawParameters = VK_TRUE;

			s_enabledFeatures.vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			s_enabledFeatures.vulkan12Features.pNext = &s_enabledFeatures.vulkan11Features;
			s_enabledFeatures.vulkan12Features.drawIndirectCount = VK_TRUE;
			s_enabledFeatures.vulkan12Features.samplerFilterMinmax = VK_TRUE;
			s_enabledFeatures.vulkan12Features.hostQueryReset = VK_TRUE;
			s_enabledFeatures.vulkan12Features.runtimeDescriptorArray = VK_TRUE;

			s_enabledFeatures.vulkan12Features.descriptorIndexing = VK_TRUE;
			s_enabledFeatures.vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
			s_enabledFeatures.vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
			s_enabledFeatures.vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
			s_enabledFeatures.vulkan12Features.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
			s_enabledFeatures.vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;

			s_enabledFeatures.vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;

			s_enabledFeatures.vulkan12Features.bufferDeviceAddress = VK_TRUE;
			s_enabledFeatures.vulkan12Features.bufferDeviceAddressCaptureReplay = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderBufferInt64Atomics = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderSharedInt64Atomics = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderFloat16 = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderInt8 = VK_TRUE;
			s_enabledFeatures.vulkan12Features.storageBuffer8BitAccess = VK_TRUE;
			s_enabledFeatures.vulkan12Features.scalarBlockLayout = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderOutputLayer = VK_TRUE;
			s_enabledFeatures.vulkan12Features.shaderOutputViewportIndex = VK_TRUE;

			s_enabledFeatures.vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			s_enabledFeatures.vulkan13Features.pNext = &s_enabledFeatures.vulkan12Features;
			s_enabledFeatures.vulkan13Features.synchronization2 = VK_TRUE;
			s_enabledFeatures.vulkan13Features.maintenance4 = VK_TRUE;
			s_enabledFeatures.vulkan13Features.dynamicRendering = VK_TRUE;
			s_enabledFeatures.vulkan13Features.shaderDemoteToHelperInvocation = VK_TRUE;

			void* chainEntryPoint = &s_enabledFeatures.vulkan13Features;

			//if (physicalDevice->IsExtensionAvailiable(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME))
			//{
			//	s_enabledFeatures.descriptorBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
			//	s_enabledFeatures.descriptorBufferFeaturesEXT.descriptorBuffer = VK_TRUE;
			//	s_enabledFeatures.descriptorBufferFeaturesEXT.descriptorBufferCaptureReplay = VK_TRUE;
			//	s_enabledFeatures.descriptorBufferFeaturesEXT.pNext = chainEntryPoint;

			//	chainEntryPoint = &s_enabledFeatures.descriptorBufferFeaturesEXT;
			//}

			if (physicalDevice->IsExtensionAvailiable(VK_EXT_MESH_SHADER_EXTENSION_NAME))
			{
				s_enabledFeatures.meshShaderFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
				s_enabledFeatures.meshShaderFeaturesEXT.meshShader = VK_TRUE;
				s_enabledFeatures.meshShaderFeaturesEXT.taskShader = VK_TRUE;
				s_enabledFeatures.meshShaderFeaturesEXT.pNext = chainEntryPoint;

				chainEntryPoint = &s_enabledFeatures.meshShaderFeaturesEXT;
			}

#ifdef VT_ENABLE_NV_AFTERMATH
			s_enabledFeatures.aftermathDiagInfo.sType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV;
			s_enabledFeatures.aftermathDiagInfo.pNext = chainEntryPoint;
			s_enabledFeatures.aftermathDiagInfo.flags =
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_ERROR_REPORTING_BIT_NV; // #TODO_Ivar: Implement some kind of debug level

			chainEntryPoint = &s_enabledFeatures.aftermathDiagInfo;
#endif

			s_enabledFeatures.physicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			s_enabledFeatures.physicalDeviceFeatures.pNext = chainEntryPoint;
			s_enabledFeatures.physicalDeviceFeatures.features.inheritedQueries = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.multiDrawIndirect = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.geometryShader = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.imageCubeArray = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.samplerAnisotropy = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.pipelineStatisticsQuery = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.fillModeNonSolid = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.wideLines = VK_TRUE;
			s_enabledFeatures.physicalDeviceFeatures.features.independentBlend = VK_TRUE;

			s_enabledFeatures.physicalDeviceFeatures.features.shaderInt16 = VK_TRUE; // #TODO_Ivar: does not work on older cards
		}

		inline static std::vector<const char*> GetEnabledExtensions(WeakPtr<VulkanPhysicalGraphicsDevice> physicalDevice)
		{
			std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

			//if (physicalDevice->IsExtensionAvailiable(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME))
			//{
			//	enabledExtensions.emplace_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
			//}

			if (physicalDevice->IsExtensionAvailiable(VK_EXT_MESH_SHADER_EXTENSION_NAME))
			{
				enabledExtensions.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
			}

#ifdef VT_ENABLE_NV_AFTERMATH
			if (physicalDevice->IsExtensionAvailiable(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
			{
				enabledExtensions.emplace_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
			}

			if (physicalDevice->IsExtensionAvailiable(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME))
			{
				enabledExtensions.emplace_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);
			}
#endif

#ifdef VT_ENABLE_VALIDATION
			if (physicalDevice->IsExtensionAvailiable(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
			{
				enabledExtensions.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
			}
#endif

			if (physicalDevice->IsExtensionAvailiable(VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME))
			{
				enabledExtensions.emplace_back(VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME);
			}

			return enabledExtensions;
		}
	}

	inline static const char* s_validationLayer = "VK_LAYER_KHRONOS_validation";

	VulkanGraphicsDevice::VulkanGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo)
	{
		m_physicalDevice = createInfo.physicalDevice.As<VulkanPhysicalGraphicsDevice>();

		VulkanPhysicalGraphicsDevice& physicalDevicePtr = m_physicalDevice->AsRef<VulkanPhysicalGraphicsDevice>();
		const auto& queueFamilies = physicalDevicePtr.GetQueueFamilies();

		std::array<VkDeviceQueueCreateInfo, 3> deviceQueueInfos{};
		constexpr std::array<float, 3> queuePriorities = { 1.f, 1.f, 1.f };

		// Graphics Queue
		{
			auto& queueCreateInfo = deviceQueueInfos[0];
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pQueuePriorities = queuePriorities.data();
			queueCreateInfo.queueFamilyIndex = queueFamilies.graphicsFamilyQueueIndex;
			queueCreateInfo.queueCount = 1;
		}

		// Compute Queue
		{
			auto& queueCreateInfo = deviceQueueInfos[1];
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pQueuePriorities = queuePriorities.data();
			queueCreateInfo.queueFamilyIndex = queueFamilies.computeFamilyQueueIndex;
			queueCreateInfo.queueCount = 1;
		}

		// Transfer Queue
		{
			auto& queueCreateInfo = deviceQueueInfos[2];
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pQueuePriorities = queuePriorities.data();
			queueCreateInfo.queueFamilyIndex = queueFamilies.transferFamilyQueueIndex;
			queueCreateInfo.queueCount = 1;
		}

		// Create Device
		{
			VkDeviceCreateInfo deviceInfo{};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueInfos.size());
			deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
			deviceInfo.pEnabledFeatures = nullptr;
			deviceInfo.enabledLayerCount = 0;

#ifdef VT_ENABLE_VALIDATION
			deviceInfo.enabledLayerCount = 1u;
			deviceInfo.ppEnabledLayerNames = &s_validationLayer;
#endif
			const auto enabledExtensions = Utility::GetEnabledExtensions(m_physicalDevice);
			Utility::GetEnabledFeatures(m_physicalDevice);

			deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
			deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
			deviceInfo.pNext = &s_enabledFeatures.physicalDeviceFeatures;

#ifdef VT_ENABLE_NV_AFTERMATH
			m_deviceCrashTracker.Initialize(GraphicsAPI::Vulkan);
#endif

			VT_VK_CHECK(vkCreateDevice(physicalDevicePtr.GetHandle<VkPhysicalDevice>(), &deviceInfo, nullptr, &m_device));
		}

		m_deviceQueues[QueueType::Graphics] = RefPtr<VulkanDeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::Graphics });
		m_deviceQueues[QueueType::TransferCopy] = RefPtr<VulkanDeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::TransferCopy });
		m_deviceQueues[QueueType::Compute] = RefPtr<VulkanDeviceQueue>::Create(DeviceQueueCreateInfo{ this, QueueType::Compute });

#ifdef VT_ENABLE_GPU_PROFILING
		{
			VkPhysicalDevice physicalDevice = m_physicalDevice.lock()->GetHandle<VkPhysicalDevice>();
			VkQueue graphicsQueue = m_deviceQueues[QueueType::Graphics]->GetHandle<VkQueue>();

			uint32_t graphicsFamily = static_cast<uint32_t>(queueFamilies.graphicsFamilyQueueIndex);

			OPTICK_GPU_INIT_VULKAN(&m_device, &physicalDevice, &graphicsQueue, &graphicsFamily, 1, nullptr);
		}
#endif
	}

	VulkanGraphicsDevice::~VulkanGraphicsDevice()
	{
		vkDestroyDevice(m_device, nullptr);
	}

	void VulkanGraphicsDevice::WaitForIdle()
	{
		// #TODO_Ivar: Must be externally synced with all queues
		vkDeviceWaitIdle(m_device);
	}

	RefPtr<DeviceQueue> VulkanGraphicsDevice::GetDeviceQueue(QueueType queueType) const
	{
		return m_deviceQueues.at(queueType);
	}

	WeakPtr<VulkanPhysicalGraphicsDevice> VulkanGraphicsDevice::GetPhysicalDevice() const
	{
		return m_physicalDevice;
	}

	void* VulkanGraphicsDevice::GetHandleImpl() const
	{
		return m_device;
	}
}

