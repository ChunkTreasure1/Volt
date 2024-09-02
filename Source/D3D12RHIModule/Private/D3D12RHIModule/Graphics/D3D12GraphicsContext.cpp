#include "dxpch.h"
#include "D3D12RHIModule/Graphics/D3D12GraphicsContext.h"
#include "D3D12RHIModule/Graphics/D3D12PhysicalGraphicsDevice.h"
#include "D3D12RHIModule/Graphics/D3D12GraphicsDevice.h"

#include "D3D12RHIModule/Memory/D3D12DefaultAllocator.h"
#include "D3D12RHIModule/Descriptors/CPUDescriptorHeapManager.h"

#include "D3D12RHIModule/Buffers/CommandSignatureCache.h"

namespace Volt::RHI
{
	namespace Utility
	{
		void LogD3D12Message(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR pDescription, void* context)
		{
			switch (severity)
			{
				case D3D12_MESSAGE_SEVERITY_CORRUPTION:
					VT_LOG(Error, std::string("D3D12 Validation:") + std::string(pDescription));
					break;
				case D3D12_MESSAGE_SEVERITY_ERROR:
					VT_LOG(Error, std::string("D3D12 Validation:") + std::string(pDescription));
					break;
				case D3D12_MESSAGE_SEVERITY_WARNING:
					VT_LOG(Warning, std::string("D3D12 Validation:") + std::string(pDescription));
					break;
				case D3D12_MESSAGE_SEVERITY_INFO:
					VT_LOG(Info, std::string("D3D12 Validation:") + std::string(pDescription));
					break;
				case D3D12_MESSAGE_SEVERITY_MESSAGE:
					VT_LOG(Trace, std::string("D3D12 Validation:") + std::string(pDescription));
					break;
				default:
					break;
			}
		}
	}

	D3D12GraphicsContext::D3D12GraphicsContext(const GraphicsContextCreateInfo& info)
		: m_createInfo(info)
	{
		Initalize();
	}

	D3D12GraphicsContext::~D3D12GraphicsContext()
	{
		Shutdown();
	}

	RefPtr<Allocator> D3D12GraphicsContext::GetDefaultAllocatorImpl()
	{
		return m_defaultAllocator;
	}

	RefPtr<Allocator> D3D12GraphicsContext::GetTransientAllocatorImpl()
	{
		return m_transientAllocator;
	}

	RefPtr<ResourceStateTracker> D3D12GraphicsContext::GetResourceStateTrackerImpl()
	{
		return m_resourceStateTracker;
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
		m_transientAllocator = TransientAllocator::Create();
		m_resourceStateTracker = RefPtr<ResourceStateTracker>::Create();
		m_cpuDescriptorHeapManager = CreateScope<CPUDescriptorHeapManager>();
		m_commandSignatureCache = CreateScope<CommandSignatureCache>();
	}

	void D3D12GraphicsContext::Shutdown()
	{
		m_commandSignatureCache = nullptr;
		m_cpuDescriptorHeapManager = nullptr;
		m_transientAllocator = nullptr;
		m_defaultAllocator = nullptr;

#ifdef VT_ENABLE_VALIDATION
		ShutdownAPIValidation();
#endif

		m_debugInterface = nullptr;
	}

	void D3D12GraphicsContext::InitializeAPIValidation()
	{
		m_infoQueue = nullptr;

		auto d3d12Device = m_graphicsDevice->GetHandle<ID3D12Device2*>();
		auto hr = (d3d12Device->QueryInterface(m_infoQueue.ReleaseAndGetAddressOf()));
		
		if (SUCCEEDED(hr))
		{
			m_infoQueue->RegisterMessageCallback(&Utility::LogD3D12Message, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_debugCallbackId);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false); 
			m_infoQueue->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_CLEANUP, true);

			D3D12_MESSAGE_SEVERITY severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			D3D12_MESSAGE_ID denyIDs[] =
			{
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_NON_OPTIMAL_BARRIER_ONLY_EXECUTE_COMMAND_LISTS
			};

			D3D12_INFO_QUEUE_FILTER queueFilter{};
			queueFilter.DenyList.NumCategories = 0;
			queueFilter.DenyList.NumSeverities = 1;
			queueFilter.DenyList.pSeverityList = severities;
			queueFilter.DenyList.NumIDs = 3;
			queueFilter.DenyList.pIDList = denyIDs;

			VT_D3D12_CHECK(m_infoQueue->PushStorageFilter(&queueFilter));
		}
		else
		{
			m_infoQueue = nullptr;
		}
	}

	void D3D12GraphicsContext::ShutdownAPIValidation()
	{
		if (m_infoQueue)
		{
			m_infoQueue->UnregisterMessageCallback(m_debugCallbackId);
		}

		m_infoQueue = nullptr;
	}

	void D3D12GraphicsContext::InitializeDebugLayer()
	{
		VT_D3D12_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface)));
		m_debugInterface->EnableDebugLayer();
	}
}
