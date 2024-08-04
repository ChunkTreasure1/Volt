#include "dxpch.h"
#include "D3D12DescriptorHeap.h"

#include "D3D12RHIModule/Graphics/D3D12GraphicsDevice.h"

#include <RHIModule/RHIProxy.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorType(D3D12DescriptorType descriptorType)
		{
			switch (descriptorType)
			{
				case D3D12DescriptorType::RTV: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				case D3D12DescriptorType::DSV: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
				case D3D12DescriptorType::CBV_SRV_UAV: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				case D3D12DescriptorType::Sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			}

			VT_ENSURE(false);
			return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		}
	}

	D3D12DescriptorHeap::D3D12DescriptorHeap(const DescriptorHeapSpecification& specification)
		: m_descriptorType(specification.descriptorType), m_maxDescriptorCount(specification.maxDescriptorCount), m_supportsGPUDescriptors(specification.supportsGPUDescriptors)
	{
		VT_ENSURE(specification.descriptorType != D3D12DescriptorType::None && "Invalid descriptor type!");
		VT_ENSURE(specification.maxDescriptorCount > 0);

		const auto& deviceProperties = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>()->GetDeviceProperties();

		switch (m_descriptorType)
		{
			case D3D12DescriptorType::RTV:
				m_descriptorSize = deviceProperties.rtvDescriptorSize;
				break;
			case D3D12DescriptorType::DSV:
				m_descriptorSize = deviceProperties.dsvDescriptorSize;
				break;
			case D3D12DescriptorType::CBV_SRV_UAV:
				m_descriptorSize = deviceProperties.cbvSrvUavDescriptorSize;
				break;
			case D3D12DescriptorType::Sampler:
				m_descriptorSize = deviceProperties.samplerDescriptorSize;
				break;
		}

		m_hash = std::hash<void*>()(reinterpret_cast<void*>(this));
		AllocateHeap(specification.maxDescriptorCount);
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		if (!m_descriptorHeap)
		{
			return;
		}

		RHIProxy::GetInstance().DestroyResource([descriptorHeap = m_descriptorHeap]() mutable
		{
			descriptorHeap = nullptr;
		});

		m_descriptorHeap = nullptr;
	}

	D3D12DescriptorPointer D3D12DescriptorHeap::Allocate(const uint32_t descriptorIndex)
	{
		VT_PROFILE_FUNCTION();
		std::scoped_lock lock{ m_mutex };

		const bool allocateAtLocation = descriptorIndex != std::numeric_limits<uint32_t>::max();

		if (!m_availiableDescriptors.empty())
		{
			if (allocateAtLocation)
			{
				auto it = std::find_if(m_availiableDescriptors.begin(), m_availiableDescriptors.end(), [&](const D3D12DescriptorPointer& descriptorPointer)
				{
					uint64_t descriptorOffset = descriptorPointer.GetCPUPointer() - m_descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
					descriptorOffset /= static_cast<uint32_t>(m_descriptorSize);

					return descriptorOffset == descriptorIndex;
				});

				if (it != m_availiableDescriptors.end())
				{
					D3D12DescriptorPointer descriptor = *it;
					m_availiableDescriptors.erase(it);
					return descriptor;
				}
			}
			else
			{
				D3D12DescriptorPointer descriptor = m_availiableDescriptors.back();
				m_availiableDescriptors.pop_back();
				return descriptor;
			}
		}

		uint32_t descriptorOffset = m_currentDescriptorCount * m_descriptorSize;

		if (allocateAtLocation)
		{
			if (descriptorIndex + 1 > m_maxDescriptorCount)
			{
				return {};
			}

			if (descriptorIndex >= m_currentDescriptorCount)
			{
				m_currentDescriptorCount = descriptorIndex + 1;
			}

			descriptorOffset = descriptorIndex * m_descriptorSize;
		}
		else
		{
			if (m_currentDescriptorCount + 1 > m_maxDescriptorCount)
			{
				return {};
			}

			m_currentDescriptorCount++;
		}


		D3D12DescriptorPointer result{};
		result.cpuPointer = m_startPointer.cpuPointer + static_cast<uint64_t>(descriptorOffset);
		
		if (m_supportsGPUDescriptors)
		{
			result.gpuPointer = m_startPointer.gpuPointer + static_cast<uint64_t>(descriptorOffset);
		}

		result.parentHeapHash = m_hash;
		return result;
	}

	void D3D12DescriptorHeap::Free(D3D12DescriptorPointer descriptor)
	{
		VT_PROFILE_FUNCTION();

		std::scoped_lock lock{ m_mutex };
		m_availiableDescriptors.emplace_back(descriptor);
	}

	void D3D12DescriptorHeap::Reset()
	{
		m_currentDescriptorCount = 0;
		m_availiableDescriptors.clear();
	}

	bool D3D12DescriptorHeap::IsAllocationSupported(D3D12DescriptorType descriptorType) const
	{
		return m_descriptorType == descriptorType && (m_currentDescriptorCount < m_maxDescriptorCount || !m_availiableDescriptors.empty());
	}

	void D3D12DescriptorHeap::AllocateHeap(uint32_t maxDescriptorCount)
	{
		VT_PROFILE_FUNCTION();

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = Utility::GetDescriptorType(m_descriptorType);
		desc.Flags = m_supportsGPUDescriptors ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = maxDescriptorCount;
		desc.NodeMask = 0;

		ID3D12Device2* d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		VT_D3D12_CHECK(d3d12Device->CreateDescriptorHeap(&desc, VT_D3D12_ID(m_descriptorHeap)));

		m_startPointer.cpuPointer = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
		
		if (m_supportsGPUDescriptors)
		{
			m_startPointer.gpuPointer = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
		}
	}
}
