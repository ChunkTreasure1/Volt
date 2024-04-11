#pragma once

#include "VoltVulkan/Core.h"
#include "VoltVulkan/Graphics/PhysicalDeviceProperties.h"

#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>

struct VkPhysicalDevice_T;

namespace Volt::RHI
{
	struct PhysicalDeviceQueueFamilyIndices
	{
		int32_t graphicsFamilyQueueIndex = -1;
		int32_t computeFamilyQueueIndex = -1;
		int32_t transferFamilyQueueIndex = -1;
	};

	struct ExtensionProperties
	{
		inline static constexpr uint32_t MAX_EXTENSION_NAME_SIZE = 256;

		char extensionName[MAX_EXTENSION_NAME_SIZE];
		uint32_t specVersion;
	};

	class VulkanPhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		VulkanPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo);
		~VulkanPhysicalGraphicsDevice() override;

		[[nodiscard]] inline std::string_view GetDeviceName() const override { return m_deviceProperties.deviceName; }
		[[nodiscard]] inline const DeviceVendor GetDeviceVendor() const override { return m_deviceProperties.vendor; }
		[[nodiscard]] inline const PhysicalDeviceProperties& GetProperties() const { return m_deviceProperties; }

		inline const PhysicalDeviceQueueFamilyIndices& GetQueueFamilies() const { return m_queueFamilyIndices; }
		const int32_t GetMemoryTypeIndex(const uint32_t reqMemoryTypeBits, const uint32_t requiredPropertyFlags);
		const bool IsExtensionAvailiable(const char* extensionName) const;
		const bool AreDescriptorBuffersEnabled() const;
		const bool AreMeshShadersEnabled() const;

	protected:
		void* GetHandleImpl() const override;

	private:
		void FetchMemoryProperties();
		void FetchDeviceProperties();
		void FetchAvailiableExtensions();

		VkPhysicalDevice_T* m_physicalDevice = nullptr;

		PhysicalDeviceQueueFamilyIndices m_queueFamilyIndices{};

		std::vector<ExtensionProperties> m_availiableExtensions;

		PhysicalDeviceProperties m_deviceProperties;
		PhysicalDeviceCreateInfo m_createInfo{};
	};

	VTVK_API Ref<PhysicalGraphicsDevice> CreateVulkanPhysicalDevice(const PhysicalDeviceCreateInfo& createInfo);
}
