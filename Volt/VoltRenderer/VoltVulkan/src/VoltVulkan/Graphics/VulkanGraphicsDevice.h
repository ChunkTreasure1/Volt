#pragma once

#include <VoltRHI/Graphics/GraphicsDevice.h>

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
		void* GetHandleImpl() override;

	private:
		VkDevice_T* m_device = nullptr;
	
		Weak<VulkanPhysicalGraphicsDevice> m_physicalDevice;
	};
}
