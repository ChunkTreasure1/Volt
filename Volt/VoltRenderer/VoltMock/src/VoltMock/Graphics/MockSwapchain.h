#pragma once
#include "VoltRHI/Graphics/Swapchain.h"

namespace Volt::RHI
{
	class MockSwapchain final : public Swapchain
	{
	public:
		MockSwapchain(GLFWwindow* glfwWindow);

		void BeginFrame() override;
		void Present() override;
		void Resize(const uint32_t width, const uint32_t height, bool useVSync) override;

		const uint32_t GetCurrentFrame() const override;
		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;

	protected:
		void* GetHandleImpl() override;
	};
}
