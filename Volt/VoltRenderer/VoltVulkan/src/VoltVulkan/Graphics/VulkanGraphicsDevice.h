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

		Ref<DeviceQueue> GetDeviceQueue(QueueType queueType) const override;
		Weak<VulkanPhysicalGraphicsDevice> GetPhysicalDevice() const;

	protected:
		void* GetHandleImpl() const override;

	private:
		VkDevice_T* m_device = nullptr;
	
		std::unordered_map<QueueType, Ref<DeviceQueue>> m_deviceQueues;

		Weak<VulkanPhysicalGraphicsDevice> m_physicalDevice;
		GPUCrashTracker m_deviceCrashTracker{};
	};
}
