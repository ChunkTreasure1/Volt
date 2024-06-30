#pragma once

#include "VoltD3D12/Common/D3D12Common.h"
#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Buffers/BufferView.h>

namespace Volt::RHI
{
	class D3D12BufferView : public BufferView
	{
	public:
		D3D12BufferView(const BufferViewSpecification& specification);
		~D3D12BufferView() override;

		VT_NODISCARD const uint64_t GetDeviceAddress() const override;

		VT_NODISCARD VT_INLINE const D3D12DescriptorPointer& GetSRVDescriptor() const { return m_srvDescriptor; }
		VT_NODISCARD VT_INLINE const D3D12DescriptorPointer& GetUAVDescriptor() const { return m_uavDescriptor; }
		VT_NODISCARD VT_INLINE const D3D12DescriptorPointer& GetCBVDescriptor() const { return m_cbvDescriptor; }
		VT_NODISCARD VT_INLINE D3D12ViewType GetD3D12ViewType() const { return m_viewType; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void CreateSRV();
		void CreateUAV();
		void CreateCBV();

		D3D12ViewType m_viewType = D3D12ViewType::None;

		D3D12DescriptorPointer m_srvDescriptor;
		D3D12DescriptorPointer m_uavDescriptor;
		D3D12DescriptorPointer m_cbvDescriptor;

		RHIResource* m_resource = nullptr;
	};
}
