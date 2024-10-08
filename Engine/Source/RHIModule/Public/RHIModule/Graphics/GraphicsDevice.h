#pragma once

#include "RHIModule/Core/RHICommon.h"
#include "RHIModule/Core/RHIInterface.h"

#include <unordered_map>
#include <mutex>

namespace Volt::RHI
{
	class DeviceQueue;
	class TransientHeap;

	class VTRHI_API GraphicsDevice : public RHIInterface
	{
	public:
		VT_DELETE_COPY_MOVE(GraphicsDevice);
		~GraphicsDevice() override = default;

		virtual RefPtr<DeviceQueue> GetDeviceQueue(QueueType queueType) const = 0;
		static RefPtr<GraphicsDevice> Create(const GraphicsDeviceCreateInfo& deviceInfo);
	
	protected:
		GraphicsDevice();
	};
}
