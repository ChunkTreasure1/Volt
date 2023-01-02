#include "vtpch.h"
#include "Swapchain.h"

#include "Volt/Log/Log.h"
#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Utility/DirectXUtils.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Core/Profiling.h"

#include <d3d11.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <chrono>

namespace Volt
{
	Swapchain::Swapchain(GLFWwindow* aWindow, Ref<GraphicsContext> aGraphicsContext)
		: myGraphicsContext(aGraphicsContext), myHeight(1), myWidth(1)
	{
		myWindowHandle = glfwGetWin32Window(aWindow);
	}

	Swapchain::~Swapchain()
	{
		Release();
		mySwapchain = nullptr;
		myGraphicsContext = nullptr;
	}

	void Swapchain::Release()
	{
		myRenderTarget = nullptr;
	}

	void Swapchain::Invalidate(uint32_t width, uint32_t height, bool aFullscreen)
	{
		VT_CORE_ASSERT(myGraphicsContext, "Graphics Context is null!");
		auto device = GraphicsContext::GetDevice();

		myWidth = width;
		myHeight = height;

		if (!mySwapchain)
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc{};
			swapChainDesc.BufferDesc.Width = width;
			swapChainDesc.BufferDesc.Height = height;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.OutputWindow = myWindowHandle;
			swapChainDesc.Windowed = !aFullscreen;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags = 0;

			IDXGIDevice* dxDevice = nullptr;
			IDXGIAdapter* adapter = nullptr;
			IDXGIFactory* factory = nullptr;
			VT_DX_CHECK(device->QueryInterface(IID_PPV_ARGS(&dxDevice)));
			VT_DX_CHECK(dxDevice->GetParent(IID_PPV_ARGS(&adapter)));
			VT_DX_CHECK(adapter->GetParent(IID_PPV_ARGS(&factory)));

			IDXGIOutput* adapterOutput;
			VT_DX_CHECK(adapter->EnumOutputs(0, &adapterOutput));

			uint32_t numModes = 0;
			adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);

			DXGI_MODE_DESC* outDesc = new DXGI_MODE_DESC[numModes];
			adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, outDesc);

			DXGI_MODE_DESC targetDesc{};

			for (uint32_t i = 0; i < numModes; i++)
			{
				if (outDesc[i].Width == width && outDesc[i].Height == height)
				{
					swapChainDesc.BufferDesc.RefreshRate = outDesc[i].RefreshRate;
				}
			}

			targetDesc = outDesc[numModes - 1];

			delete[] outDesc;

			VT_DX_CHECK(factory->CreateSwapChain(dxDevice, &swapChainDesc, mySwapchain.GetAddressOf()));

			myMonitorMode.height = targetDesc.Height;
			myMonitorMode.width = targetDesc.Width;
			myMonitorMode.refreshRate = (float)targetDesc.RefreshRate.Denominator / (float)targetDesc.RefreshRate.Numerator;

			device->Release();
			adapter->Release();
			factory->Release();
		}

		Release();

		if (width == 0 || height == 0)
		{
			return;
		}

		VT_DX_CHECK(mySwapchain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

		if (myIsFullscreen != aFullscreen)
		{
			VT_DX_CHECK(mySwapchain->SetFullscreenState(aFullscreen, nullptr));
			myIsFullscreen = aFullscreen;
		}

		ID3D11Texture2D* backBuffer;
		VT_DX_CHECK(mySwapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		VT_DX_CHECK(device->CreateRenderTargetView(backBuffer, nullptr, &myRenderTarget));

		backBuffer->Release();

		Bind();
	}

	void Swapchain::BeginFrame()
	{
		auto context = GraphicsContext::GetImmediateContext();

		const float color[4] = { 0.f, 0.f, 0.f, 1.f };
		context->ClearRenderTargetView(myRenderTarget.Get(), color);
	}

	void Swapchain::Present(bool aUseVSync)
	{
		VT_PROFILE_FUNCTION();
		VT_DX_CHECK(mySwapchain->Present(aUseVSync ? 1 : 0, 0));

		Bind();
	}

	void Swapchain::Bind() const
	{
		auto context = GraphicsContext::GetImmediateContext();

		D3D11_VIEWPORT viewport{};
		viewport.Width = (float)myWidth;
		viewport.Height = (float)myHeight;
		viewport.MaxDepth = 1.f;
		viewport.MinDepth = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.TopLeftX = 0.f;

		context->OMSetRenderTargets(1, myRenderTarget.GetAddressOf(), nullptr);
		context->RSSetViewports(1, &viewport);
	}

	void Swapchain::Resize(uint32_t width, uint32_t height, bool aFullscreen)
	{
		Invalidate(width, height, aFullscreen);
	}

	Ref<Swapchain> Swapchain::Create(GLFWwindow* aWindow, Ref<GraphicsContext> aGraphicsContext)
	{
		return CreateScope<Swapchain>(aWindow, aGraphicsContext);
	}
}