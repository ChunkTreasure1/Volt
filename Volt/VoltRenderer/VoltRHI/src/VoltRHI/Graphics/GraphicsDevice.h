#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include <unordered_map>
#include <mutex>

namespace Volt::RHI
{
	class DeviceQueue;
	class TransientHeap;

	class VTRHI_API GraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(GraphicsDevice);
		~GraphicsDevice() override = default;

		virtual RefPtr<DeviceQueue> GetDeviceQueue(QueueType queueType) const = 0;
		static RefPtr<GraphicsDevice> Create(const GraphicsDeviceCreateInfo& deviceInfo);
	
	protected:
		GraphicsDevice();
	};
}
