#pragma once

#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Graphics/DeviceQueue.h"

namespace Volt
{
	struct GraphicsDeviceInfo
	{

	};

	class GraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(GraphicsDevice);
		~GraphicsDevice() override = default;

		static Ref<GraphicsDevice> Create(const GraphicsDeviceInfo& deviceInfo);

		Ref<CommandBuffer> GetSingleUseCommandBuffer(QueueType queueType);
		void ExecuteSingleUseCommandBuffer(Ref<CommandBuffer> commandBuffer);

	protected:
		GraphicsDevice();

		std::unordered_map<QueueType, Ref<DeviceQueue>> m_deviceQueues;
		std::unordered_map<QueueType, std::mutex> m_deviceQueueMutexes;
	};
}
