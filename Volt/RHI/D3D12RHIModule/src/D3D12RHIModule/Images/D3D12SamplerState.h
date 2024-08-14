#pragma once

#include "D3D12RHIModule/Descriptors/DescriptorCommon.h"

#include <RHIModule/Images/SamplerState.h>

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
