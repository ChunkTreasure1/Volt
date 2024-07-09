#include "dxpch.h"
#include "CPUDescriptorHeapManager.h"

#include "VoltD3D12/Descriptors/D3D12DescriptorHeap.h"

namespace Volt::RHI
{
	constexpr uint32_t DEFAULT_HEAP_DESCRIPTOR_COUNT = 500;

	CPUDescriptorHeapManager::CPUDescriptorHeapManager()
	{
		Initialize();
	}

	CPUDescriptorHeapManager::~CPUDescriptorHeapManager()
	{
		m_descriptorHeaps.clear();
	}

	D3D12DescriptorPointer CPUDescriptorHeapManager::Allocate(D3D12DescriptorType descriptorType)
	{
		std::scoped_lock lock{ m_mutex };

		for (const auto& heap : m_descriptorHeaps)
		{
			if (heap->IsAllocationSupported(descriptorType))
			{
				return heap->Allocate();
			}
		}

		m_descriptorHeaps.push_back(CreateDescriptorHeap(descriptorType));
		return m_descriptorHeaps.back()->Allocate();
	}

	void CPUDescriptorHeapManager::Free(D3D12DescriptorPointer descriptorPointer)
	{
		std::scoped_lock lock{ m_mutex };
		
		for (const auto& heap : m_descriptorHeaps)
		{
			if (heap->GetHash() == descriptorPointer.parentHeapHash)
			{
				heap->Free(descriptorPointer);
				return;
			}
		}

		VT_ENSURE(false);
	}

	D3D12DescriptorHeap& CPUDescriptorHeapManager::GetHeapFromHash(size_t hash) const
	{
		for (const auto& heap : m_descriptorHeaps)
		{
			if (heap->GetHash() == hash)
			{
				return *heap;
			}
		}

		VT_ENSURE(false);

		static D3D12DescriptorHeap nullHeap({});
		return nullHeap;
	}

	void CPUDescriptorHeapManager::Initialize()
	{
		m_descriptorHeaps.push_back(CreateDescriptorHeap(D3D12DescriptorType::RTV));
		m_descriptorHeaps.push_back(CreateDescriptorHeap(D3D12DescriptorType::DSV));
		m_descriptorHeaps.push_back(CreateDescriptorHeap(D3D12DescriptorType::CBV_SRV_UAV));
		m_descriptorHeaps.push_back(CreateDescriptorHeap(D3D12DescriptorType::Sampler));
	}

	Scope<D3D12DescriptorHeap> CPUDescriptorHeapManager::CreateDescriptorHeap(D3D12DescriptorType descriptorType)
	{
		DescriptorHeapSpecification specification{};
		specification.descriptorType = descriptorType;
		specification.maxDescriptorCount = DEFAULT_HEAP_DESCRIPTOR_COUNT;
		specification.supportsGPUDescriptors = false;

		return CreateScope<D3D12DescriptorHeap>(specification);
	}
}
