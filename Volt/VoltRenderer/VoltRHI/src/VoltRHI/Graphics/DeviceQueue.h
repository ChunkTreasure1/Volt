#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class CommandBuffer;

	class DeviceQueue : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(DeviceQueue);

		virtual void WaitForQueue() = 0;
		virtual void Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer) = 0;

		static Ref<DeviceQueue> Create(const DeviceQueueCreateInfo& createInfo);

	protected:
		DeviceQueue() = default;

		QueueType m_queueType = QueueType::Graphics;
	};
}
