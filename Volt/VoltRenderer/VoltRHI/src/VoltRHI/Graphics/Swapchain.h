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

		VT_NODISCARD uint32_t GetCurrentFrameIndex() const { return m_currentFrameIndex; }

	protected:
		Swapchain() = default;
		uint32_t m_currentFrameIndex;
	};
}
