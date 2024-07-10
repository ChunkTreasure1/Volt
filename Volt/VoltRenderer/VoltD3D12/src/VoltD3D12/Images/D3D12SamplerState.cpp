#include "dxpch.h"
#include "D3D12SamplerState.h"

#include "VoltD3D12/Common/D3D12Helpers.h"
#include "VoltD3D12/Descriptors/CPUDescriptorHeapManager.h"
#include "VoltD3D12/Graphics/D3D12GraphicsContext.h"

#include <VoltRHI/RHIProxy.h>

namespace Volt::RHI
{
	D3D12SamplerState::D3D12SamplerState(const SamplerStateCreateInfo& createInfo)
	{
		D3D12_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = Utility::VoltToD3D12Filter(createInfo.minFilter, createInfo.magFilter, createInfo.mipFilter, createInfo.compareOperator);
		samplerDesc.AddressU = Utility::VoltToD3D12WrapMode(createInfo.wrapMode);
		samplerDesc.AddressV = Utility::VoltToD3D12WrapMode(createInfo.wrapMode);
		samplerDesc.AddressW = Utility::VoltToD3D12WrapMode(createInfo.wrapMode);
		samplerDesc.MipLODBias = createInfo.mipLodBias;
		samplerDesc.MinLOD = createInfo.minLod;
		samplerDesc.MaxLOD = createInfo.maxLod;
	
		samplerDesc.BorderColor[0] = 1.f;
		samplerDesc.BorderColor[1] = 1.f;
		samplerDesc.BorderColor[2] = 1.f;
		samplerDesc.BorderColor[3] = 1.f;
	
		samplerDesc.ComparisonFunc = Utility::VoltToD3D12CompareOp(createInfo.compareOperator);
		samplerDesc.MaxAnisotropy = static_cast<uint32_t>(createInfo.anisotropyLevel);

		m_samplerDescriptor = GraphicsContext::Get().As<D3D12GraphicsContext>()->GetCPUDescriptorHeapManager().Allocate(D3D12DescriptorType::Sampler);
		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		d3d12Device->CreateSampler(&samplerDesc, D3D12_CPU_DESCRIPTOR_HANDLE(m_samplerDescriptor.GetCPUPointer()));
	}
	
	D3D12SamplerState::~D3D12SamplerState()
	{
		RHIProxy::GetInstance().DestroyResource([descriptor = m_samplerDescriptor]() 
		{
			GraphicsContext::Get().As<D3D12GraphicsContext>()->GetCPUDescriptorHeapManager().Free(descriptor);
		});
	}
	
	void* D3D12SamplerState::GetHandleImpl() const
	{
		return reinterpret_cast<void*>(&m_samplerDescriptor);
	}
}
