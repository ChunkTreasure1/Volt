#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Graphics/DeviceQueue.h"

#include <unordered_map>
#include <mutex>

namespace Volt::RHI
{
	class GraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(GraphicsDevice);
		~GraphicsDevice() override = default;

		inline Ref<DeviceQueue> GetDeviceQueue(QueueType queueType) const { return m_deviceQueues.at(queueType); }
		static Ref<GraphicsDevice> Create(const GraphicsDeviceCreateInfo& deviceInfo);
	
	protected:
		GraphicsDevice();

		std::unordered_map<QueueType, Ref<DeviceQueue>> m_deviceQueues;
	};
}
