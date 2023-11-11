#pragma once

#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Utility/GPUCrashTracker.h>

struct VkDevice_T;

namespace Volt::RHI
{
	class VulkanPhysicalGraphicsDevice;

	class VulkanGraphicsDevice final : public GraphicsDevice
	{
	public:
		VulkanGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo);
		~VulkanGraphicsDevice() override;

		void WaitForIdle();

		Weak<VulkanPhysicalGraphicsDevice> GetPhysicalDevice() const;

	protected:
		void* GetHandleImpl() const override;

	private:
		VkDevice_T* m_device = nullptr;
	
		Weak<VulkanPhysicalGraphicsDevice> m_physicalDevice;
		GPUCrashTracker m_deviceCrashTracker{};
	};
}
