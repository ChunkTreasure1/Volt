#include "vkpch.h"
#include "VulkanPhysicalGraphicsDevice.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanGraphicsContext.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	namespace Utility
	{
		const std::pair<VkPhysicalDevice, VkPhysicalDeviceProperties> FindBestSuitableDevice(VkInstance instance)
		{
			uint32_t deviceCount = 0;
			VT_VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

			if (deviceCount == 0)
			{
				throw std::runtime_error("[PhysicalDevice] Failed to find GPU with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices{ deviceCount };
			VT_VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

			VkPhysicalDevice nonDiscreteDevice = nullptr;

			VkPhysicalDeviceProperties deviceProperties{};
			for (const auto& device : devices)
			{
				vkGetPhysicalDeviceProperties(device, &deviceProperties);

				if (deviceProperties.apiVersion == VK_API_VERSION_1_3)
				{
					if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					{
						return { device, deviceProperties };
					}
				
					nonDiscreteDevice = device;
				}
			}

			if (!nonDiscreteDevice)
			{
				throw std::runtime_error("[PhysicalDevice] Failed to find GPU with Vulkan 1.3 support!");
			}

			return { nonDiscreteDevice, deviceProperties };
		}

		const PhysicalDeviceQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyCount };
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

			PhysicalDeviceQueueFamilyIndices result{};

			for (int32_t i = 0; const auto& queueFamily : queueFamilyProperties)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && result.graphicsFamilyQueueIndex == -1)
				{
					result.graphicsFamilyQueueIndex = i;
					i++;
					continue;
				}

				if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && result.computeFamilyQueueIndex == -1)
				{
					result.computeFamilyQueueIndex = i;
					i++;
					continue;
				}

				if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && result.transferFamilyQueueIndex== -1)
				{
					result.transferFamilyQueueIndex = i;
					i++;
					continue;
				}

				if (result.computeFamilyQueueIndex != -1 &&
					result.graphicsFamilyQueueIndex != -1 &&
					result.transferFamilyQueueIndex != -1)
				{
					break;
				}

				i++;
			}

			if (result.computeFamilyQueueIndex == -1)
			{
				result.computeFamilyQueueIndex = result.graphicsFamilyQueueIndex;
			}

			if (result.transferFamilyQueueIndex == -1)
			{
				result.transferFamilyQueueIndex = result.graphicsFamilyQueueIndex;
			}

			return result;
		}

		static DeviceVendor VulkanVendorIDToVendor(uint32_t vendorID)
		{
			switch (vendorID)
			{
				case 0x10DE: return DeviceVendor::NVIDIA;
				case 0x1002: return DeviceVendor::AMD;
				case 0x8086: return DeviceVendor::Intel;
			}
			return DeviceVendor::Unknown;
		}
	}

	VulkanPhysicalGraphicsDevice::VulkanPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo)
		: m_createInfo(createInfo)
	{
		VkInstance vulkanInstance = GraphicsContext::Get().As<VulkanGraphicsContext>()->GetInstance();

		const auto [selectedDevice, deviceProperties] = Utility::FindBestSuitableDevice(vulkanInstance);
		m_physicalDevice = selectedDevice;

		m_capabilities.gpuName = deviceProperties.deviceName;
		m_capabilities.deviceVendor = Utility::VulkanVendorIDToVendor(deviceProperties.vendorID);

		m_queueFamilyIndices = Utility::FindQueueFamilyIndices(selectedDevice);
	}

	VulkanPhysicalGraphicsDevice::~VulkanPhysicalGraphicsDevice()
	{
	}

	void* VulkanPhysicalGraphicsDevice::GetHandleImpl()
	{
		return m_physicalDevice;
	}
}
