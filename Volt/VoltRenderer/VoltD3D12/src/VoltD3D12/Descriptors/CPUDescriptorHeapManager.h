#pragma once

#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <CoreUtilities/Core.h>

#include <vector>

namespace Volt::RHI
{
	class D3D12DescriptorHeap;
	class CPUDescriptorHeapManager
	{
	public:
		CPUDescriptorHeapManager();
		~CPUDescriptorHeapManager();

		D3D12DescriptorPointer Allocate(D3D12DescriptorType descriptorType);
		void Free(D3D12DescriptorPointer descriptorPointer);

	private:
		void Initialize();
		Scope<D3D12DescriptorHeap> CreateDescriptorHeap(D3D12DescriptorType descriptorType);

		std::mutex m_mutex;
		std::vector<Scope<D3D12DescriptorHeap>> m_descriptorHeaps;
	};
}
