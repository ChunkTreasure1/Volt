#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Utility/GPUCrashTracker.h>

struct VkDevice_T;

namespace Volt::RHI
{
	class VulkanPhysicalGraphicsDevice;

	class VulkanGraphicsDevice final : public GraphicsDevice
	{
	public:
		VulkanGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo);
		~VulkanGraphicsDevice() override;

		void WaitForIdle();

		RefPtr<DeviceQueue> GetDeviceQueue(QueueType queueType) const override;
		WeakPtr<VulkanPhysicalGraphicsDevice> GetPhysicalDevice() const;

	protected:
		void* GetHandleImpl() const override;

	private:
		VkDevice_T* m_device = nullptr;
	
		std::unordered_map<QueueType, RefPtr<DeviceQueue>> m_deviceQueues;

		WeakPtr<VulkanPhysicalGraphicsDevice> m_physicalDevice;
		GPUCrashTracker m_deviceCrashTracker{};
	};
}
