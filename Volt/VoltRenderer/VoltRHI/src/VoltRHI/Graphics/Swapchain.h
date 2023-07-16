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
		virtual void Resize(const uint32_t width, const uint32_t height, bool useVSync) = 0;

		VT_NODISCARD virtual const uint32_t GetCurrentFrame() const = 0;
		VT_NODISCARD virtual const uint32_t GetWidth() const = 0;
		VT_NODISCARD virtual const uint32_t GetHeight() const = 0;

		static Ref<Swapchain> Create(GLFWwindow* window);

	protected:
		Swapchain() = default;
	};
}
