#include "dxpch.h"
#include "DescriptorUtility.h"

#include "D3D12RHIModule/Graphics/D3D12GraphicsContext.h"
#include "D3D12RHIModule/Descriptors/CPUDescriptorHeapManager.h"

namespace Volt::RHI::DescriptorUtility
{
	D3D12DescriptorPointer AllocateDescriptorPointer(D3D12DescriptorType type)
	{
		return GraphicsContext::Get().AsRef<D3D12GraphicsContext>().GetCPUDescriptorHeapManager().Allocate(type);
	}

	void FreeDescriptorPointer(D3D12DescriptorPointer descriptorPointer)
	{
		GraphicsContext::Get().AsRef<D3D12GraphicsContext>().GetCPUDescriptorHeapManager().Free(descriptorPointer);
	}
}
