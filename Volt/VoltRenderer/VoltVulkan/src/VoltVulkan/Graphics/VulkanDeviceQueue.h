#pragma once

#include <VoltRHI/Graphics/DeviceQueue.h>

struct VkQueue_T;

namespace Volt::RHI
{
	class VulkanDeviceQueue final : public DeviceQueue
	{
	public:
		VulkanDeviceQueue(const DeviceQueueCreateInfo& createInfo);
		~VulkanDeviceQueue() override;

		void WaitForQueue() override;
		void Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		std::mutex m_executeMutex{};

		VkQueue_T* m_queue = nullptr;
	};
}
