#include "dxpch.h"
#include "D3D12GraphicsContext.h"
#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

#include "VoltD3D12/Memory/D3D12DefaultAllocator.h"

namespace Volt::RHI
{
	D3D12GraphicsContext::D3D12GraphicsContext(const GraphicsContextCreateInfo& info)
		: m_createInfo(info)
	{
		Initalize();
	}

	D3D12GraphicsContext::~D3D12GraphicsContext()
	{
		Shutdown();
	}

	Allocator& D3D12GraphicsContext::GetDefaultAllocatorImpl()
	{
		return *m_defaultAllocator;
	}

	RefPtr<Allocator> D3D12GraphicsContext::GetTransientAllocatorImpl()
	{
		return nullptr;
	}

	RefPtr<GraphicsDevice> D3D12GraphicsContext::GetGraphicsDevice() const
	{
		return m_graphicsDevice;
	}

	RefPtr<PhysicalGraphicsDevice> D3D12GraphicsContext::GetPhysicalGraphicsDevice() const
	{
		return m_physicalDevice;
	}

	void* D3D12GraphicsContext::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12GraphicsContext::Initalize()
	{
		m_physicalDevice = PhysicalGraphicsDevice::Create(m_createInfo.physicalDeviceInfo);

		GraphicsDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.physicalDevice = m_physicalDevice;

#ifdef VT_ENABLE_VALIDATION
		InitializeDebugLayer();
#endif

		m_graphicsDevice = GraphicsDevice::Create(deviceCreateInfo);

#ifdef VT_ENABLE_VALIDATION
		InitializeAPIValidation();
#endif

		m_defaultAllocator = DefaultAllocator::Create();
	}

	void D3D12GraphicsContext::Shutdown()
	{
		m_infoQueue = nullptr;
		m_debugInterface = nullptr;
	}

	void D3D12GraphicsContext::InitializeAPIValidation()
	{
		m_infoQueue = nullptr;

		auto d3d12Device = m_graphicsDevice->GetHandle<ID3D12Device2*>();
		auto hr = (d3d12Device->QueryInterface(m_infoQueue.ReleaseAndGetAddressOf()));
		
		if (SUCCEEDED(hr))
		{
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			m_infoQueue->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_CLEANUP, true);
		}
	}

	void D3D12GraphicsContext::InitializeDebugLayer()
	{
		VT_D3D12_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface)));
		m_debugInterface->EnableDebugLayer();
	}
}
