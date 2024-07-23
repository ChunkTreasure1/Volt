#pragma once

#include "VoltD3D12/Descriptors/DescriptorCommon.h"
#include "VoltD3D12/Common/ComPtr.h"

#include <vector>

struct ID3D12DescriptorHeap;

namespace Volt::RHI
{
	struct DescriptorHeapSpecification
	{
		D3D12DescriptorType descriptorType;
		uint32_t maxDescriptorCount;
		bool supportsGPUDescriptors = false;
	};

	class D3D12DescriptorHeap
	{
	public:
		D3D12DescriptorHeap(const DescriptorHeapSpecification& specification);
		~D3D12DescriptorHeap();

		D3D12DescriptorPointer Allocate(const uint32_t descriptorIndex = std::numeric_limits<uint32_t>::max());
		void Free(D3D12DescriptorPointer descriptor);

		// This will invalidate all allocated descriptors
		void Reset();

		bool IsAllocationSupported(D3D12DescriptorType descriptorType) const;
		VT_NODISCARD VT_INLINE size_t GetHash() const { return m_hash; }
		VT_NODISCARD VT_INLINE ComPtr<ID3D12DescriptorHeap> GetHeap() const { return m_descriptorHeap; }

	private:
		void AllocateHeap(uint32_t maxDescriptorCount);

		D3D12DescriptorType m_descriptorType = D3D12DescriptorType::None;
		size_t m_hash = 0;
		uint32_t m_descriptorSize = 0;
		uint32_t m_maxDescriptorCount = 0;
		bool m_supportsGPUDescriptors = false;

		D3D12DescriptorPointer m_startPointer{};
		Vector<D3D12DescriptorPointer> m_availiableDescriptors;
		uint32_t m_currentDescriptorCount = 0;

		std::mutex m_mutex;
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	};
}
