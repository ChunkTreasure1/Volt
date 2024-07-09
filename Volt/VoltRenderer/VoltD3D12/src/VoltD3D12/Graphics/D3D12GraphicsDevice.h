#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Graphics/GraphicsDevice.h>

struct ID3D12Device2;
struct ID3D12DebugDevice;

namespace Volt::RHI
{
	struct GraphicsDeviceCreateInfo;

	class D3D12GraphicsDevice final : public GraphicsDevice
	{
	public:
		struct Properties
		{
			uint32_t rtvDescriptorSize;
			uint32_t dsvDescriptorSize;
			uint32_t cbvSrvUavDescriptorSize;
			uint32_t samplerDescriptorSize;
		};

		D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info);
		~D3D12GraphicsDevice() override;

		RefPtr<DeviceQueue> GetDeviceQueue(QueueType queueType) const override;

		VT_NODISCARD VT_INLINE const Properties& GetDeviceProperties() const { return m_properties; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void InitializeProperties();

		Properties m_properties;

		std::unordered_map<QueueType, RefPtr<DeviceQueue>> m_deviceQueues;
		ComPtr<ID3D12Device2> m_device;
		ComPtr<ID3D12DebugDevice> m_debugDevice;
	};
}
