#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

struct GLFWwindow;

namespace Volt::RHI
{
	class Image2D;
	class Swapchain : public RHIInterface
	{
	public:
		VT_DELETE_COMMON_OPERATORS(Swapchain);
		~Swapchain() override = default;

		virtual void BeginFrame() = 0;
		virtual void Present() = 0;
		virtual void Resize(const uint32_t width, const uint32_t height, bool enableVSync) = 0;

		VT_NODISCARD virtual const uint32_t GetCurrentFrame() const = 0;
		VT_NODISCARD virtual Ref<Image2D> GetCurrentImage() const = 0;
		VT_NODISCARD virtual const uint32_t GetWidth() const = 0;
		VT_NODISCARD virtual const uint32_t GetHeight() const = 0;
		VT_NODISCARD virtual const uint32_t GetFramesInFlight() const = 0;
		VT_NODISCARD virtual const PixelFormat GetFormat() const = 0;

		static Ref<Swapchain> Create(GLFWwindow* window);

	protected:
		Swapchain() = default;
	};
}
