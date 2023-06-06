#pragma once
#include "VoltRHI/Graphics/Swapchain.h"

namespace Volt
{
	class MockSwapchain final : public Swapchain
	{
	public:
		MockSwapchain(GLFWwindow* glfwWindow);

		// Inherited via Swapchain
		void* GetHandleImpl() override;

		void BeginFrame() override;

		void Present() override;

	};
}
