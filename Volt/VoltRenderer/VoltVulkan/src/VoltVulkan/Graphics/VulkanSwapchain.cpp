#include "vkpch.h"
#include "VulkanSwapchain.h"

namespace Volt
{
	VulkanSwapchain::VulkanSwapchain(GLFWwindow* glfwWindow)
	{
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
	}

	void VulkanSwapchain::BeginFrame()
	{
	}

	void VulkanSwapchain::Present()
	{
	
	}

	void VulkanSwapchain::Resize(const uint32_t width, const uint32_t height, bool useVSync)
	{
	}

	const uint32_t VulkanSwapchain::GetCurrentFrame() const
	{
		return 0;
	}
	
	const uint32_t VulkanSwapchain::GetWidth() const
	{
		return 0;
	}
	
	const uint32_t VulkanSwapchain::GetHeight() const
	{
		return 0;
	}
	
	void* VulkanSwapchain::GetHandleImpl()
	{
		return m_swapchain;
	}
}
