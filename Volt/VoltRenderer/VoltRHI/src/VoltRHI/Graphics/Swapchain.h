#pragma once

#include "VoltRHI/Core/RHIInterface.h"

struct GLFWwindow;

namespace Volt
{
	class Swapchain : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(Swapchain);
		~Swapchain() override = default;

		virtual void BeginFrame() = 0;
		virtual void Present() = 0;
		virtual void Resize(const uint32_t width, const uint32_t height, bool useVSync);

		static Ref<Swapchain> Create(GLFWwindow* window);

	protected:
		Swapchain() = default;
	};
}
