#pragma once

#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <CoreUtilities/Core.h>



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

		D3D12DescriptorHeap& GetHeapFromHash(size_t hash) const;

	private:
		void Initialize();
		Scope<D3D12DescriptorHeap> CreateDescriptorHeap(D3D12DescriptorType descriptorType);

		std::mutex m_mutex;
		Vector<Scope<D3D12DescriptorHeap>> m_descriptorHeaps;
	};
}
