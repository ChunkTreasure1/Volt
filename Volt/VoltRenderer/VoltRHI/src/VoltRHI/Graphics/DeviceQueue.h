#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt
{
	class CommandBuffer;

	class DeviceQueue : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(DeviceQueue);

		virtual void WaitForQueue() = 0;
		virtual void Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer) = 0;

	protected:
		QueueType m_queueType = QueueType::Graphics;
	};
}
