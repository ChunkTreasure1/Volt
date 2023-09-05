#include "dxpch.h"
#include "D3D12GraphicsContext.h"
#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

#include "VoltD3D12/Common/D3D12Allocator.h"

namespace Volt::RHI
{
	D3D12GraphicsContext::D3D12GraphicsContext(const GraphicsContextCreateInfo& info)
	{
		Initalize(info);
	}

	D3D12GraphicsContext::~D3D12GraphicsContext()
	{
		Shutdown();
	}

	void* D3D12GraphicsContext::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12GraphicsContext::Initalize(const GraphicsContextCreateInfo& info)
	{
		m_physicalDevice = PhysicalGraphicsDevice::Create(info.physicalDeviceInfo);

		GraphicsDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.physicalDevice = m_physicalDevice;

		// enable Debuging.
		VT_D3D12_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug)));
		m_debug->EnableDebugLayer();

		m_graphicsDevice = GraphicsDevice::Create(deviceCreateInfo);

		if (CreateAPIDebugging())
		{
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

			m_infoQueue->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_CLEANUP, true);
		}

		D3d12Allocator::Initialize();
	}

	void D3D12GraphicsContext::Shutdown()
	{
		VT_D3D12_DELETE(m_debug);
	}

	const std::vector<Ref<TransientHeap>>& D3D12GraphicsContext::GetTransientHeaps() const
	{
		static std::vector<Ref<TransientHeap>> heaps;
		return heaps;
	}

	bool D3D12GraphicsContext::CreateAPIDebugging()
	{
		

		auto d3d12GraphicsDevice = m_graphicsDevice->As<D3D12GraphicsDevice>();
		auto d3d12Device = d3d12GraphicsDevice->GetHandle<ID3D12Device2*>();

		m_infoQueue = nullptr;

		auto hr = (d3d12Device->QueryInterface(&m_infoQueue));

		if (FAILED(hr) || !m_infoQueue)
		{
			return false;
		}

		return true;
	}
}
