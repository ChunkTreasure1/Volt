#include "mkpch.h"
#include "MockSwapchain.h"

namespace Volt
{
    MockSwapchain::MockSwapchain(GLFWwindow* glfwWindow)
    {
    }

    void* MockSwapchain::GetHandleImpl()
    {
        return nullptr;
    }

    void MockSwapchain::BeginFrame()
    {
    }

    void MockSwapchain::Present()
    {
    }
	
	void MockSwapchain::Resize(const uint32_t width, const uint32_t height, bool useVSync)
	{
	}

	const uint32_t MockSwapchain::GetCurrentFrame() const
	{
		return 0;
	}

	const uint32_t MockSwapchain::GetWidth() const
	{
		return 0;
	}

	const uint32_t MockSwapchain::GetHeight() const
	{
		return 0;
	}
}
