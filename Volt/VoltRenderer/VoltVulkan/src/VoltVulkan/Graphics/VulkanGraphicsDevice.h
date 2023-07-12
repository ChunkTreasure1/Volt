#pragma once

#include <VoltRHI/Graphics/GraphicsDevice.h>

struct VkDevice_T;

namespace Volt
{
	class VulkanGraphicsDevice final : public GraphicsDevice
	{
	public:
		VulkanGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo);
		~VulkanGraphicsDevice() override;

	protected:
		void* GetHandleImpl() override;

	private:
		VkDevice_T* m_device = nullptr;
	};
}
