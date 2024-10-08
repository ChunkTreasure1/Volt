#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"

#include <RHIModule/Graphics/GraphicsDevice.h>

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

		struct Capabilities
		{
			bool supportsEnhancedBarriers = false;
		};

		D3D12GraphicsDevice(const GraphicsDeviceCreateInfo& info);
		~D3D12GraphicsDevice() override;

		RefPtr<DeviceQueue> GetDeviceQueue(QueueType queueType) const override;

		VT_NODISCARD VT_INLINE const Properties& GetDeviceProperties() const { return m_properties; }
		VT_NODISCARD VT_INLINE const Capabilities& GetDeviceCapabilities() const { return m_capabilities; }

		uint64_t GetAndIncreaseFenceValue();
		uint64_t GetFenceValue();

	protected:
		void* GetHandleImpl() const override;

	private:
		void InitializeProperties();
		void InitializeCapabilities();

		Properties m_properties;
		Capabilities m_capabilities;

		std::unordered_map<QueueType, RefPtr<DeviceQueue>> m_deviceQueues;
		ComPtr<ID3D12Device10> m_device;
		ComPtr<ID3D12DebugDevice> m_debugDevice;

		uint64_t m_currentFenceValue = 0;
	};
}
