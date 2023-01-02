#pragma once

#include "Volt/Core/Base.h"

#include <Windows.h>
#include <wrl.h>

struct GLFWwindow;
struct ID3D11RenderTargetView;
struct IDXGISwapChain;

using namespace Microsoft::WRL;

namespace Volt
{
	class GraphicsContext;
	class Swapchain
	{
	public:
		Swapchain(GLFWwindow* aWindow, Ref<GraphicsContext> aGraphicsContext);
		~Swapchain();

		void Release();
		void Invalidate(uint32_t width, uint32_t height, bool aFullscreen);

		void BeginFrame();
		void Present(bool aUseVSync);
		void Bind() const;

		void Resize(uint32_t width, uint32_t height, bool aFullscreen);
		
		const auto& GetMonitorMode() const { return myMonitorMode; }
		
		static Ref<Swapchain> Create(GLFWwindow* aWindow, Ref<GraphicsContext> aGraphicsContext);

	private:
		struct MonitorMode
		{
			uint32_t width = 0;
			uint32_t height = 0;
			float refreshRate = 0.f;
		};

		uint32_t myWidth = 0;
		uint32_t myHeight = 0;

		bool myIsFullscreen = false;
		bool myFirstFrame = true;

		float myAverageGPUTimingStart = 0.f;
		uint32_t myAverageGPUTimingFrames = 0;

		Ref<GraphicsContext> myGraphicsContext;

		HWND myWindowHandle = 0;
		MonitorMode myMonitorMode{};

		ComPtr<ID3D11RenderTargetView> myRenderTarget = nullptr;
		ComPtr<IDXGISwapChain> mySwapchain = nullptr;
	};
}