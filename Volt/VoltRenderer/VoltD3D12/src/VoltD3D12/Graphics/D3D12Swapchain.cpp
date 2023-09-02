#include "dxpch.h"
#include "D3D12Swapchain.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "VoltD3D12/Common/D3D12DescriptorHeapManager.h"

#include "VoltD3D12/Graphics/D3D12DeviceQueue.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"

namespace Volt::RHI
{
	D3D12Swapchain::D3D12Swapchain(GLFWwindow* window)
	{
		m_windowHandle = window;
		m_width = 1280;
		m_height = 720;
		m_enableVsync = false;
		Build();
	}

	D3D12Swapchain::~D3D12Swapchain()
	{
		auto d3d12Queue = GraphicsContext::GetDevice()->GetDeviceQueue(QueueType::Graphics)->As<D3D12DeviceQueue>();
		for (auto& fence : m_fences)
		{
			fence.Increment();
			d3d12Queue->Signal(fence, fence.Value());
			fence.Wait();
		}
		CleanUp();
		for (auto& target : m_renderTargets)
		{
			delete target.view;
			target.view = nullptr;
			target.hasID = false;
		}
	}

	void* D3D12Swapchain::GetHandleImpl() const
	{
		return nullptr;
	}
	void D3D12Swapchain::BeginFrame()
	{
		m_currentImageIndex = m_swapchain->GetCurrentBackBufferIndex();
		m_fences[m_currentImageIndex].Wait();
	}

	void D3D12Swapchain::Present()
	{
		m_swapchain->Present(m_enableVsync, DXGI_PRESENT_ALLOW_TEARING);
		auto d3d12Queue = GraphicsContext::GetDevice()->GetDeviceQueue(QueueType::Graphics)->As<D3D12DeviceQueue>();
		d3d12Queue->Signal(m_fences[m_currentImageIndex], m_fences[m_currentImageIndex].Value());
		m_fences[m_currentImageIndex].Signal();
		m_fences[m_currentImageIndex].Increment();

	}

	void D3D12Swapchain::Resize(const uint32_t width, const uint32_t height, bool enableVSync)
	{
		m_width = width;
		m_height = height;
		m_enableVsync = enableVSync;

		auto d3d12Queue = GraphicsContext::GetDevice()->GetDeviceQueue(QueueType::Graphics)->As<D3D12DeviceQueue>();

		for (auto& fence : m_fences)
		{
			fence.Increment();
			d3d12Queue->Signal(fence, fence.Value());
			fence.Wait();
		}

		for (auto& target : m_renderTargets)
		{
			VT_D3D12_DELETE(target.resource);
		}

		VT_D3D12_CHECK(m_swapchain->ResizeBuffers(MaxSwapchainImages, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));

		for (size_t i = 0; i < MaxSwapchainImages; i++)
		{
			m_swapchain->GetBuffer(static_cast<UINT>(i), VT_D3D12_ID(m_renderTargets[i].resource));

			auto name = L"SwapchainTarget - Flight ID[" + std::to_wstring(i) + L"]";
			m_renderTargets[i].resource->SetName(name.c_str());

			if (m_renderTargets[i].hasID)
			{
				D3D12DescriptorHeapManager::CreateRTVHandleFromID(*m_renderTargets[i].view, m_renderTargets[i].id);
			}

			auto d3d12Device = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>();
			d3d12Device->GetHandle<ID3D12Device2*>()->CreateRenderTargetView(m_renderTargets[i].resource, nullptr, *m_renderTargets[i].view);
		}
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
		return MaxSwapchainImages;
	}

	void D3D12Swapchain::Build()
	{
		auto d3d12Device = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>();
		auto d3d12Queue = GraphicsContext::GetDevice()->GetDeviceQueue(QueueType::Graphics)->As<D3D12DeviceQueue>();
		auto d3d12PhysicalDevice = GraphicsContext::GetPhysicalDevice()->As<D3D12PhysicalGraphicsDevice>();

		DXGI_SWAP_CHAIN_DESC scDesc{};

		DXGI_MODE_DESC mDesc{};
		mDesc.Width = static_cast<uint32_t>(m_width);
		mDesc.Height = static_cast<uint32_t>(m_height);

		DXGI_RATIONAL ratio{};

		/*if (specs.refreshRate > 0)
		{
			ratio.Numerator = 1;
			ratio.Denominator = specs.refreshRate;
		}*/

		mDesc.RefreshRate = ratio;
		mDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC sDesc{};
		sDesc.Count = 1;

		scDesc.BufferDesc = mDesc;
		scDesc.SampleDesc = sDesc;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = MaxSwapchainImages;
		scDesc.OutputWindow = glfwGetWin32Window(m_windowHandle); 
		scDesc.Windowed = true;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		Microsoft::WRL::ComPtr<IDXGISwapChain> tranferSwapchain = nullptr;

		auto queue = d3d12Queue->GetHandle<ID3D12CommandQueue*>();

		VT_D3D12_CHECK(d3d12PhysicalDevice->GetFactory()->CreateSwapChain(queue, &scDesc, tranferSwapchain.GetAddressOf()));


		tranferSwapchain->QueryInterface(&m_swapchain);


		m_currentImageIndex = m_swapchain->GetCurrentBackBufferIndex();


		for (size_t i = 0; i < MaxSwapchainImages; i++)
		{
			m_swapchain->GetBuffer(static_cast<UINT>(i), VT_D3D12_ID(m_renderTargets[i].resource));

			auto name = L"SwapchainTarget - Flight ID[" + std::to_wstring(i) + L"]";
			m_renderTargets[i].resource->SetName(name.c_str());

			m_renderTargets[i].view = new CD3DX12_CPU_DESCRIPTOR_HANDLE;
			m_renderTargets[i].id = D3D12DescriptorHeapManager::CreateNewRTVHandle(*m_renderTargets[i].view);
			m_renderTargets[i].hasID = true;


			d3d12Device->GetHandle<ID3D12Device2*>()->CreateRenderTargetView(m_renderTargets[i].resource, nullptr, *m_renderTargets[i].view);

			m_fences[i].Create(QueueType::Graphics);
			m_fences[i].Signal();
			m_fences[i].Increment();
		}
	}

	void D3D12Swapchain::CleanUp()
	{
		for (auto& target : m_renderTargets)
		{
			VT_D3D12_DELETE(target.resource);
		}

		VT_D3D12_DELETE(m_swapchain);
	}
}
