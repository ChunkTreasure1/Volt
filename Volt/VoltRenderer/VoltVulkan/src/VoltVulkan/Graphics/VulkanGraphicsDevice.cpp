#include "vkpch.h"
#include "VulkanGraphicsDevice.h"

namespace Volt
{
	VulkanGraphicsDevice::VulkanGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo)
	{
	}
	
	VulkanGraphicsDevice::~VulkanGraphicsDevice()
	{
	}

	void* VulkanGraphicsDevice::GetHandleImpl()
	{
		return m_device;
	}
}
