#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"

namespace Volt::RHI
{
	class CommandBuffer;
	class Semaphore;
	class Fence;

	struct DeviceQueueExecuteInfo
	{
		Vector<WeakPtr<CommandBuffer>> commandBuffers;
		Vector<WeakPtr<Semaphore>> signalSemaphores;
	
		WeakPtr<Fence> fence;
	};

	class VTRHI_API DeviceQueue : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(DeviceQueue);

		virtual void WaitForQueue() = 0;
		virtual void Execute(const DeviceQueueExecuteInfo& executeInfo) = 0;

		static RefPtr<DeviceQueue> Create(const DeviceQueueCreateInfo& createInfo);

	protected:
		DeviceQueue() = default;

		QueueType m_queueType = QueueType::Graphics;
	};
}
