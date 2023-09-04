#include "vkpch.h"
#include "VulkanPhysicalGraphicsDevice.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanGraphicsContext.h"

#include <vulkan/vulkan.h>

namespace Volt::RHI
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

				constexpr auto VERSION = VK_API_VERSION_1_3;

				if (deviceProperties.apiVersion >= VERSION)
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
		VkInstance vulkanInstance = GraphicsContext::Get().AsRef<VulkanGraphicsContext>().GetInstance();

		auto [selectedDevice, deviceProperties] = Utility::FindBestSuitableDevice(vulkanInstance);

		m_physicalDevice = selectedDevice;

		m_capabilities.gpuName = deviceProperties.deviceName;
		m_capabilities.deviceVendor = Utility::VulkanVendorIDToVendor(deviceProperties.vendorID);
		m_capabilities.timestampSupport = deviceProperties.limits.timestampComputeAndGraphics;
		m_capabilities.timestampPeriod = deviceProperties.limits.timestampPeriod;
		m_capabilities.maxAnisotropyLevel = deviceProperties.limits.maxSamplerAnisotropy;

		m_queueFamilyIndices = Utility::FindQueueFamilyIndices(selectedDevice);

		FindMemoryProperties();
	}

	VulkanPhysicalGraphicsDevice::~VulkanPhysicalGraphicsDevice()
	{
	}

	const int32_t VulkanPhysicalGraphicsDevice::GetMemoryTypeIndex(const uint32_t reqMemoryTypeBits, const uint32_t requiredPropertyFlags)
	{
		for (const auto& memoryType : m_deviceMemoryProperties.memoryTypes)
		{
			const uint32_t memTypeBits = (1 << memoryType.index);
			const bool isRequiredMemoryType = reqMemoryTypeBits & memTypeBits;
		
			const VkMemoryPropertyFlags properties = static_cast<VkMemoryPropertyFlags>(memoryType.propertyFlags);
			const bool hasRequiredProperties = (properties & requiredPropertyFlags) == requiredPropertyFlags;

			if (isRequiredMemoryType && hasRequiredProperties)
			{
				return static_cast<int32_t>(memoryType.index);
			}
		}

		return -1;
	}

	void* VulkanPhysicalGraphicsDevice::GetHandleImpl() const
	{
		return m_physicalDevice;
	}

	void VulkanPhysicalGraphicsDevice::FindMemoryProperties()
	{
		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			auto& memType = m_deviceMemoryProperties.memoryTypes.emplace_back();
			memType.heapIndex = memoryProperties.memoryTypes[i].heapIndex;
			memType.propertyFlags = memoryProperties.memoryTypes[i].propertyFlags;
			memType.index = i;
		}

		for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			auto& memHeap = m_deviceMemoryProperties.memoryHeaps.emplace_back();
			memHeap.flags = memoryProperties.memoryHeaps[i].flags;
			memHeap.size = memoryProperties.memoryHeaps[i].size;
		}
	}
}
