#pragma once
#include "VoltRHI/Graphics/DeviceQueue.h"
namespace Volt
{
	class MockDeviceQueue final : public DeviceQueue
	{
	public:
		void* GetHandleImpl() override;
		void WaitForQueue() override;
		void Execute(const std::vector<Ref<CommandBuffer>>& commandBuffer) override;
	};
}

