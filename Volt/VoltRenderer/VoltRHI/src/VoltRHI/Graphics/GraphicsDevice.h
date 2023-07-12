#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Graphics/DeviceQueue.h"

#include <unordered_map>
#include <mutex>

namespace Volt
{
	class GraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(GraphicsDevice);
		~GraphicsDevice() override = default;

		static Ref<GraphicsDevice> Create(const GraphicsDeviceCreateInfo& deviceInfo);

		Ref<CommandBuffer> GetSingleUseCommandBuffer(QueueType queueType);
		void ExecuteSingleUseCommandBuffer(Ref<CommandBuffer> commandBuffer);

	protected:
		GraphicsDevice();

		std::unordered_map<QueueType, Ref<DeviceQueue>> m_deviceQueues;
		std::unordered_map<QueueType, std::mutex> m_deviceQueueMutexes;
	};
}
