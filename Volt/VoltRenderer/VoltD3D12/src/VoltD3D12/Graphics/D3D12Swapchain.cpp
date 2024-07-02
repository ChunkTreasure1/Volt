#include "dxpch.h"
#include "D3D12Swapchain.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"

#include "VoltD3D12/Descriptors/DescriptorUtility.h"

namespace Volt::RHI
{
	namespace Utility
	{
		inline bool GetSupportsTearing()
		{
			bool result = false;

			ComPtr<IDXGIFactory4> factory4;
			if (SUCCEEDED(CreateDXGIFactory1(VT_D3D12_ID(factory4))))
			{
				ComPtr<IDXGIFactory5> factory5;
				if (SUCCEEDED(factory4.As(&factory5)))
				{
					if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &result, sizeof(result))))
					{
						result = false;
					}
				}
			}

			return result;
		}
	}

	D3D12Swapchain::D3D12Swapchain(GLFWwindow* window)
	{
		m_windowHandle = window;
		m_commandBuffer = CommandBuffer::Create(GetFramesInFlight());

		Invalidate(m_width, m_height, m_enableVsync);
	}

	D3D12Swapchain::~D3D12Swapchain()
	{
		Release();
	}

	void* D3D12Swapchain::GetHandleImpl() const
	{
		return m_swapchain.Get();
	}

	void D3D12Swapchain::BeginFrame()
	{
		VT_PROFILE_FUNCTION();

		m_currentImageIndex = m_swapchain->GetCurrentBackBufferIndex();
		m_commandBuffer->Begin();
	}

	void D3D12Swapchain::Present()
	{
		VT_PROFILE_FUNCTION();

		{
			ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Image;
			barrier.imageBarrier().srcAccess = BarrierAccess::RenderTarget;
			barrier.imageBarrier().srcStage = BarrierStage::RenderTarget;
			barrier.imageBarrier().srcLayout = m_perImageData.at(m_currentImageIndex).imageReference->GetImageLayout();
			barrier.imageBarrier().dstAccess = BarrierAccess::None;
			barrier.imageBarrier().dstStage = BarrierStage::AllGraphics;
			barrier.imageBarrier().dstLayout = ImageLayout::Present;
			barrier.imageBarrier().resource = m_perImageData.at(m_currentImageIndex).imageReference;

			m_commandBuffer->ResourceBarrier({ barrier });
		}

		m_commandBuffer->End();
		m_commandBuffer->Execute();

		VT_D3D12_CHECK(m_swapchain->Present(m_enableVsync, m_supportsTearing ? DXGI_PRESENT_ALLOW_TEARING : 0));
	}

	void D3D12Swapchain::Resize(const uint32_t width, const uint32_t height, bool enableVSync)
	{
		VT_PROFILE_FUNCTION();

		m_width = width;
		m_height = height;
		m_enableVsync = enableVSync;

		{
			m_commandBuffer->WaitForFences();
		}

		for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
		{
			m_perImageData[i].imageReference = nullptr;
			m_perImageData[i].resource.Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		VT_D3D12_CHECK(m_swapchain->GetDesc(&swapChainDesc));
		VT_D3D12_CHECK(m_swapchain->ResizeBuffers(MAX_SWAPCHAIN_IMAGES, m_width, m_height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		GetSwapchainImages();
	}

	VT_NODISCARD const uint32_t D3D12Swapchain::GetCurrentFrame() const
	{
		return m_currentImageIndex;
	}

	VT_NODISCARD const uint32_t D3D12Swapchain::GetWidth() const
	{
		return m_width;
	}

	VT_NODISCARD const uint32_t D3D12Swapchain::GetHeight() const
	{
		return m_height;
	}

	const uint32_t D3D12Swapchain::GetFramesInFlight() const
	{
		return MAX_SWAPCHAIN_IMAGES;
	}

	RefPtr<Image2D> D3D12Swapchain::GetCurrentImage() const
	{
		return m_perImageData.at(m_currentImageIndex).imageReference;
	}

	RefPtr<CommandBuffer> D3D12Swapchain::GetCommandBuffer() const
	{
		return RefPtr<CommandBuffer>();
	}

	const PixelFormat D3D12Swapchain::GetFormat() const
	{
		return PixelFormat::R8G8B8A8_UNORM;
	}

	void D3D12Swapchain::Invalidate(const uint32_t width, const uint32_t height, bool enableVSync)
	{
		CreateSwapchain(width, height);
		GetSwapchainImages();
	}

	void D3D12Swapchain::Release()
	{
	}

	void D3D12Swapchain::CreateSwapchain(const uint32_t width, const uint32_t height)
	{
		uint32_t factoryFlags = 0;

#ifdef VT_ENABLE_VALIDATION
		factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ComPtr<IDXGIFactory4> factory;
		VT_D3D12_CHECK(CreateDXGIFactory2(factoryFlags, VT_D3D12_ID(factory)));

		m_supportsTearing = Utility::GetSupportsTearing();

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
		swapchainDesc.Width = width;
		swapchainDesc.Height = height;
		swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.Stereo = false;
		swapchainDesc.SampleDesc = { 1, 0 };
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = MAX_SWAPCHAIN_IMAGES;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = m_supportsTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ID3D12CommandQueue* cmdQueue = GraphicsContext::GetDevice()->GetDeviceQueue(QueueType::Graphics)->GetHandle<ID3D12CommandQueue*>();
		HWND windowsWindowHandle = glfwGetWin32Window(m_windowHandle);

		ComPtr<IDXGISwapChain1> swapchain1;
		VT_D3D12_CHECK(factory->CreateSwapChainForHwnd(cmdQueue, windowsWindowHandle, &swapchainDesc, nullptr, nullptr, &swapchain1));
		VT_D3D12_CHECK(factory->MakeWindowAssociation(windowsWindowHandle, DXGI_MWA_NO_ALT_ENTER));
		VT_D3D12_CHECK(swapchain1.As(&m_swapchain));
	}

	void D3D12Swapchain::GetSwapchainImages()
	{
		for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
		{
			ComPtr<ID3D12Resource> backBuffer;
			m_swapchain->GetBuffer(i, VT_D3D12_ID(backBuffer));
		
			const std::wstring name = L"Swapchain Target - Index " + std::to_wstring(i);
			backBuffer->SetName(name.c_str());

			m_perImageData[i].resource = backBuffer;
		
			SwapchainImageSpecification spec{};
			spec.swapchain = this;
			spec.imageIndex = i;
			m_perImageData[i].imageReference = Image2D::Create(spec);
		}
	}
}
