#pragma once

#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Images/SamplerState.h>

namespace Volt::RHI
{
	class D3D12SamplerState : public SamplerState
	{
	public: 
		D3D12SamplerState(const SamplerStateCreateInfo& createInfo);
		~D3D12SamplerState() override;

	protected:
		void* GetHandleImpl() const override;

	private:
		mutable D3D12DescriptorPointer m_samplerDescriptor;
	};
}
