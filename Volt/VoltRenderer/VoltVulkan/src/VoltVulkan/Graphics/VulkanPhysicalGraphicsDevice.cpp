#include "vkpch.h"
#include "VulkanPhysicalGraphicsDevice.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanGraphicsContext.h"

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		const VkPhysicalDevice FindBestSuitableDevice(VkInstance instance)
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
						return device;
					}
				
					nonDiscreteDevice = device;
				}
			}

			if (!nonDiscreteDevice)
			{
				throw std::runtime_error("[PhysicalDevice] Failed to find GPU with Vulkan 1.3 support!");
			}

			return nonDiscreteDevice;
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
		VkPhysicalDevice selectedDevice = Utility::FindBestSuitableDevice(vulkanInstance);

		m_physicalDevice = selectedDevice;
		m_queueFamilyIndices = Utility::FindQueueFamilyIndices(selectedDevice);

		FetchAvailiableExtensions();
		FetchDeviceProperties();
	}

	VulkanPhysicalGraphicsDevice::~VulkanPhysicalGraphicsDevice()
	{
	}

	const int32_t VulkanPhysicalGraphicsDevice::GetMemoryTypeIndex(const uint32_t reqMemoryTypeBits, const uint32_t requiredPropertyFlags)
	{
		for (const auto& memoryType : m_deviceProperties.memoryProperties.memoryTypes)
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

	const bool VulkanPhysicalGraphicsDevice::IsExtensionAvailiable(const char* extensionName) const
	{
		for (const auto& ext : m_availiableExtensions)
		{
			if (strcmp(extensionName, ext.extensionName) == 0)
			{
				return true;
			}
		}

		return false;
	}

	const bool VulkanPhysicalGraphicsDevice::AreDescriptorBuffersEnabled() const
	{
		return m_deviceProperties.descriptorBufferProperties.enabled;
	}

	void* VulkanPhysicalGraphicsDevice::GetHandleImpl() const
	{
		return m_physicalDevice;
	}

	void VulkanPhysicalGraphicsDevice::FetchMemoryProperties()
	{
		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			auto& memType = m_deviceProperties.memoryProperties.memoryTypes.emplace_back();
			memType.heapIndex = memoryProperties.memoryTypes[i].heapIndex;
			memType.propertyFlags = memoryProperties.memoryTypes[i].propertyFlags;
			memType.index = i;
		}

		for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			auto& memHeap = m_deviceProperties.memoryProperties.memoryHeaps.emplace_back();
			memHeap.flags = memoryProperties.memoryHeaps[i].flags;
			memHeap.size = memoryProperties.memoryHeaps[i].size;
		}
	}

	void VulkanPhysicalGraphicsDevice::FetchDeviceProperties()
	{
		void* firstChainPtr = nullptr;

		VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};
		descriptorBufferProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
		descriptorBufferProperties.pNext = firstChainPtr;
		firstChainPtr = &descriptorBufferProperties;


		VkPhysicalDeviceProperties2	deviceProperties{};
		deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		deviceProperties.pNext = firstChainPtr;

		vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);

		m_deviceProperties.deviceName = deviceProperties.properties.deviceName;
		m_deviceProperties.vendor = Utility::VulkanVendorIDToVendor(deviceProperties.properties.vendorID);
		m_deviceProperties.hasTimestampSupport = deviceProperties.properties.limits.timestampComputeAndGraphics;
		m_deviceProperties.timestampPeriod = deviceProperties.properties.limits.timestampPeriod;
		m_deviceProperties.maxAnisotropyLevel = deviceProperties.properties.limits.maxSamplerAnisotropy;

		// VK_EXT_descriptor_buffer
		{
			m_deviceProperties.descriptorBufferProperties.enabled = IsExtensionAvailiable(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
			m_deviceProperties.descriptorBufferProperties.combinedImageSamplerDescriptorSingleArray = descriptorBufferProperties.combinedImageSamplerDescriptorSingleArray;
			m_deviceProperties.descriptorBufferProperties.bufferlessPushDescriptors = descriptorBufferProperties.bufferlessPushDescriptors;
			m_deviceProperties.descriptorBufferProperties.allowSamplerImageViewPostSubmitCreation = descriptorBufferProperties.allowSamplerImageViewPostSubmitCreation;
			m_deviceProperties.descriptorBufferProperties.descriptorBufferOffsetAlignment = descriptorBufferProperties.descriptorBufferOffsetAlignment;
			m_deviceProperties.descriptorBufferProperties.maxDescriptorBufferBindings = descriptorBufferProperties.maxDescriptorBufferBindings;
			m_deviceProperties.descriptorBufferProperties.maxResourceDescriptorBufferBindings = descriptorBufferProperties.maxResourceDescriptorBufferBindings;
			m_deviceProperties.descriptorBufferProperties.maxSamplerDescriptorBufferBindings = descriptorBufferProperties.maxSamplerDescriptorBufferBindings;
			m_deviceProperties.descriptorBufferProperties.maxEmbeddedImmutableSamplerBindings = descriptorBufferProperties.maxEmbeddedImmutableSamplerBindings;
			m_deviceProperties.descriptorBufferProperties.maxEmbeddedImmutableSamplers = descriptorBufferProperties.maxEmbeddedImmutableSamplers;
			m_deviceProperties.descriptorBufferProperties.bufferCaptureReplayDescriptorDataSize = descriptorBufferProperties.bufferCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.imageCaptureReplayDescriptorDataSize = descriptorBufferProperties.imageCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.imageViewCaptureReplayDescriptorDataSize = descriptorBufferProperties.imageViewCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.samplerCaptureReplayDescriptorDataSize = descriptorBufferProperties.samplerCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.accelerationStructureCaptureReplayDescriptorDataSize = descriptorBufferProperties.accelerationStructureCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.samplerDescriptorSize = descriptorBufferProperties.samplerDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.combinedImageSamplerDescriptorSize = descriptorBufferProperties.combinedImageSamplerDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.sampledImageDescriptorSize = descriptorBufferProperties.sampledImageDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.storageImageDescriptorSize = descriptorBufferProperties.storageImageDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.uniformTexelBufferDescriptorSize = descriptorBufferProperties.uniformTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustUniformTexelBufferDescriptorSize = descriptorBufferProperties.robustUniformTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.storageTexelBufferDescriptorSize = descriptorBufferProperties.storageTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustStorageTexelBufferDescriptorSize = descriptorBufferProperties.robustStorageTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.uniformBufferDescriptorSize = descriptorBufferProperties.uniformBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustUniformBufferDescriptorSize = descriptorBufferProperties.robustUniformBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.storageBufferDescriptorSize = descriptorBufferProperties.storageBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustStorageBufferDescriptorSize = descriptorBufferProperties.robustStorageBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.inputAttachmentDescriptorSize = descriptorBufferProperties.inputAttachmentDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.accelerationStructureDescriptorSize = descriptorBufferProperties.accelerationStructureDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.maxSamplerDescriptorBufferRange = descriptorBufferProperties.maxSamplerDescriptorBufferRange;
			m_deviceProperties.descriptorBufferProperties.maxResourceDescriptorBufferRange = descriptorBufferProperties.maxResourceDescriptorBufferRange;
			m_deviceProperties.descriptorBufferProperties.samplerDescriptorBufferAddressSpaceSize = descriptorBufferProperties.samplerDescriptorBufferAddressSpaceSize;
			m_deviceProperties.descriptorBufferProperties.resourceDescriptorBufferAddressSpaceSize = descriptorBufferProperties.resourceDescriptorBufferAddressSpaceSize;
			m_deviceProperties.descriptorBufferProperties.descriptorBufferAddressSpaceSize = descriptorBufferProperties.descriptorBufferAddressSpaceSize;
		}

		FetchMemoryProperties();
	}

	void VulkanPhysicalGraphicsDevice::FetchAvailiableExtensions()
	{
		uint32_t extCount = 0;
		VT_VK_CHECK(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr));
		
		m_availiableExtensions.resize(extCount);
		VT_VK_CHECK(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, reinterpret_cast<VkExtensionProperties*>(m_availiableExtensions.data())));
	}
}
