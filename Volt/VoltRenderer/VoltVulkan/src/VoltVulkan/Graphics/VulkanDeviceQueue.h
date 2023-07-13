#pragma once

#include <VoltRHI/Graphics/DeviceQueue.h>

struct VkQueue_T;

namespace Volt
{
	class VulkanDeviceQueue final : public DeviceQueue
	{
	public:
		VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo);
		~VulkanDeviceQueue() override;

	protected:
		void* GetHandleImpl() override;

	private:
		VkQueue_T* m_queue = nullptr;
	};
}
