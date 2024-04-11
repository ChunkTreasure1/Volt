#pragma once
#include "VoltRHI/Graphics/GraphicsContext.h"

struct ID3D12InfoQueue;
struct ID3D12Debug;

namespace Volt::RHI
{
	class D3D12GraphicsContext final : public GraphicsContext
	{
	public:
		D3D12GraphicsContext(const GraphicsContextCreateInfo& info);
		~D3D12GraphicsContext() override;

	protected:
		Allocator& GetDefaultAllocatorImpl() override;
		Ref<Allocator> GetTransientAllocatorImpl() override;
		Ref<GraphicsDevice> GetGraphicsDevice() const override;
		Ref<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const override;

		void* GetHandleImpl() const override;

	private:
		void Initalize(const GraphicsContextCreateInfo& info);
		void Shutdown();

		bool CreateAPIDebugging();

		Ref<GraphicsDevice> m_graphicsDevice;
		Ref<PhysicalGraphicsDevice> m_physicalDevice;

		Scope<Allocator> m_defaultAllocator;
		Ref<Allocator> m_transientAllocator;

		ID3D12InfoQueue* m_infoQueue;
		ID3D12Debug* m_debug;
	};
}
