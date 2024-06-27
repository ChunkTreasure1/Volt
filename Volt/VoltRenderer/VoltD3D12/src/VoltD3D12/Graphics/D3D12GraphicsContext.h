#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Graphics/GraphicsContext.h>

struct ID3D12InfoQueue;
struct ID3D12Debug;

namespace Volt::RHI
{
	class CPUDescriptorHeapManager;
	class D3D12GraphicsContext final : public GraphicsContext
	{
	public:
		D3D12GraphicsContext(const GraphicsContextCreateInfo& info);
		~D3D12GraphicsContext() override;

		CPUDescriptorHeapManager& GetCPUDescriptorHeapManager() const { return *m_cpuDescriptorHeapManager; }

	protected:
		Allocator& GetDefaultAllocatorImpl() override;
		RefPtr<Allocator> GetTransientAllocatorImpl() override;
		RefPtr<GraphicsDevice> GetGraphicsDevice() const override;
		RefPtr<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const override;

		void* GetHandleImpl() const override;

	private:
		void Initalize();
		void Shutdown();

		void InitializeAPIValidation();
		void ShutdownAPIValidation();
		void InitializeDebugLayer();

		RefPtr<GraphicsDevice> m_graphicsDevice;
		RefPtr<PhysicalGraphicsDevice> m_physicalDevice;

		Scope<Allocator> m_defaultAllocator;
		RefPtr<Allocator> m_transientAllocator;

		Scope<CPUDescriptorHeapManager> m_cpuDescriptorHeapManager;

		GraphicsContextCreateInfo m_createInfo{};

		ComPtr<ID3D12InfoQueue1> m_infoQueue;
		unsigned long m_debugCallbackId = 0;

		ComPtr<ID3D12Debug> m_debugInterface;
	};
}
