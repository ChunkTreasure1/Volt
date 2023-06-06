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
}
