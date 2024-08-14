#pragma once

#include "VulkanRHIModule/Core.h"
#include <RHIModule/Graphics/DeviceQueue.h>

struct VkQueue_T;

namespace Volt::RHI
{
	class VulkanDeviceQueue final : public DeviceQueue
	{
	public:
		VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo);
		~VulkanDeviceQueue() override;

		void WaitForQueue() override;
		void Execute(const DeviceQueueExecuteInfo& commandBuffer) override;

		void AquireLock();
		void ReleaseLock();

	protected:
		void* GetHandleImpl() const override;

	private:
		std::mutex m_executeMutex{};

		VkQueue_T* m_queue = nullptr;
	};
}
