#pragma once

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

	class VulkanPhysicalGraphicsDevice final : public PhysicalGraphicsDevice
	{
	public:
		VulkanPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo);
		~VulkanPhysicalGraphicsDevice() override;

		inline const PhysicalDeviceQueueFamilyIndices& GetQueueFamilies() const { return m_queueFamilyIndices; }

	protected:
		void* GetHandleImpl() const override;

	private:
		VkPhysicalDevice_T* m_physicalDevice = nullptr;

		PhysicalDeviceQueueFamilyIndices m_queueFamilyIndices{};
		PhysicalDeviceCreateInfo m_createInfo{};
	};
}
