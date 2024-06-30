#include "dxpch.h"
#include "D3D12DescriptorTable.h"

#include "VoltD3D12/Descriptors/D3D12DescriptorHeap.h"
#include "VoltD3D12/Images/D3D12ImageView.h"
#include "VoltD3D12/Buffers/D3D12CommandBuffer.h"
#include "VoltD3D12/Buffers/D3D12BufferView.h"

#include <VoltRHI/Images/ImageUtility.h>

namespace Volt::RHI
{
	D3D12DescriptorTable::D3D12DescriptorTable(const DescriptorTableCreateInfo& createInfo)
	{
		m_shader = createInfo.shader;

		Invalidate();
	}

	D3D12DescriptorTable::~D3D12DescriptorTable()
	{
		Release();
	}

	void D3D12DescriptorTable::SetImageView(WeakPtr<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_allocatedDescriptorPointers[set].contains(binding))
		{
			RHILog::LogTagged(LogSeverity::Warning, "[D3D12DescriptorTable]:", "Trying to assign image view at set {0} and binding {1}. But that is not a valid binding!", set, binding);
			return;
		}

		m_isDirty = true;
	
		const auto& descriptorInfo = m_allocatedDescriptorPointers.at(set).at(binding);
		const auto& d3d12View = imageView->AsRef<D3D12ImageView>();

		if ((d3d12View.GetD3D12ViewType() & descriptorInfo.viewType) == D3D12ViewType::None)
		{
			RHILog::LogTagged(LogSeverity::Error, "[D3D12DescriptorTable]:", "Image View does not support the required D3D12 view type!");
			return;
		}

		auto& descriptorCopy = m_activeDescriptorCopies.emplace_back();
		descriptorCopy.dstPointer = descriptorInfo.pointer;

		if ((descriptorInfo.viewType & D3D12ViewType::SRV) != D3D12ViewType::None)
		{
			descriptorCopy.srcPointer = d3d12View.GetSRVDescriptor();
		}
		else
		{
			descriptorCopy.srcPointer = d3d12View.GetUAVDescriptor();
		}
	}

	void D3D12DescriptorTable::SetBufferView(WeakPtr<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
		if (!m_allocatedDescriptorPointers[set].contains(binding))
		{
			RHILog::LogTagged(LogSeverity::Warning, "[D3D12DescriptorTable]:", "Trying to assign buffer view at set {0} and binding {1}. But that is not a valid binding!", set, binding);
			return;
		}

		m_isDirty = true;

		const auto& descriptorInfo = m_allocatedDescriptorPointers.at(set).at(binding);
		const auto& d3d12View = bufferView->AsRef<D3D12BufferView>();

		if ((d3d12View.GetD3D12ViewType() & descriptorInfo.viewType) == D3D12ViewType::None)
		{
			RHILog::LogTagged(LogSeverity::Error, "[D3D12DescriptorTable]:", "Buffer View does not support the required D3D12 view type!");
			return;
		}

		auto& descriptorCopy = m_activeDescriptorCopies.emplace_back();
		descriptorCopy.dstPointer = descriptorInfo.pointer;

		if ((descriptorInfo.viewType & D3D12ViewType::SRV) != D3D12ViewType::None)
		{
			descriptorCopy.srcPointer = d3d12View.GetSRVDescriptor();
		}
		else
		{
			descriptorCopy.srcPointer = d3d12View.GetUAVDescriptor();
		}
	}

	void D3D12DescriptorTable::SetSamplerState(WeakPtr<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex)
	{
	}

	void D3D12DescriptorTable::SetImageView(std::string_view name, WeakPtr<ImageView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetImageView(view, binding.set, binding.binding, arrayIndex);
	}

	void D3D12DescriptorTable::SetBufferView(std::string_view name, WeakPtr<BufferView> view, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetBufferView(view, binding.set, binding.binding, arrayIndex);
	}

	void D3D12DescriptorTable::SetSamplerState(std::string_view name, WeakPtr<SamplerState> samplerState, uint32_t arrayIndex)
	{
		const auto& binding = m_shader->GetResourceBindingFromName(name);
		if (!binding.IsValid())
		{
			return;
		}

		SetSamplerState(samplerState, binding.set, binding.binding, arrayIndex);
	}

	void D3D12DescriptorTable::PrepareForRender()
	{
		if (!m_isDirty)
		{
			return;
		}

		if (m_activeDescriptorCopies.empty() && m_activeSamplerDescriptorCopies.empty())
		{
			return;
		}

		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
	
		if (!m_activeDescriptorCopies.empty())
		{
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srcHandles(m_activeDescriptorCopies.size());
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dstHandles(m_activeDescriptorCopies.size());
			std::vector<uint32_t> copySizes(m_activeDescriptorCopies.size(), 1u);

			for (size_t i = 0; i < m_activeDescriptorCopies.size(); i++)
			{
				srcHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeDescriptorCopies.at(i).srcPointer.GetCPUPointer());
				dstHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeDescriptorCopies.at(i).dstPointer.GetCPUPointer());
			}

			d3d12Device->CopyDescriptors(static_cast<uint32_t>(m_activeDescriptorCopies.size()), dstHandles.data(), copySizes.data(), static_cast<uint32_t>(m_activeDescriptorCopies.size()), srcHandles.data(), copySizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		if (!m_activeSamplerDescriptorCopies.empty())
		{
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srcHandles(m_activeSamplerDescriptorCopies.size());
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dstHandles(m_activeSamplerDescriptorCopies.size());
			std::vector<uint32_t> copySizes(m_activeSamplerDescriptorCopies.size(), 1u);

			for (size_t i = 0; i < m_activeSamplerDescriptorCopies.size(); i++)
			{
				srcHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeSamplerDescriptorCopies.at(i).srcPointer.GetCPUPointer());
				dstHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeSamplerDescriptorCopies.at(i).dstPointer.GetCPUPointer());
			}

			d3d12Device->CopyDescriptors(static_cast<uint32_t>(m_activeSamplerDescriptorCopies.size()), dstHandles.data(), copySizes.data(), static_cast<uint32_t>(m_activeSamplerDescriptorCopies.size()), srcHandles.data(), copySizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		}

		m_activeDescriptorCopies.clear();
		m_activeSamplerDescriptorCopies.clear();
		m_isDirty = false;
	}

	void D3D12DescriptorTable::Bind(CommandBuffer& commandBuffer)
	{
		ID3D12GraphicsCommandList* cmdList = commandBuffer.GetHandle<ID3D12GraphicsCommandList*>();
		ID3D12DescriptorHeap* heaps[2] {};
		uint32_t heapCount = 0;

		if (m_mainHeap)
		{
			heaps[heapCount++] = m_mainHeap->GetHeap().Get();
		}

		if (m_samplerHeap)
		{
			heaps[heapCount++] = m_samplerHeap->GetHeap().Get();
		}

		cmdList->SetDescriptorHeaps(heapCount, heaps);
	}

	void* D3D12DescriptorTable::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12DescriptorTable::Invalidate()
	{
		VT_PROFILE_FUNCTION();
		Release();

		const auto& resources = m_shader->GetResources();

		constexpr uint32_t PUSH_CONSTANT_BINDING = 999;

		uint32_t mainDescriptorCount = 0;
		uint32_t samplerDescriptorCount = 0;

		for (const auto& [set, bindings] : resources.uniformBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				if (binding == PUSH_CONSTANT_BINDING)
				{
					continue;
				}

				auto& info = m_allocatedDescriptorPointers[set][binding];
				info.viewType = D3D12ViewType::CBV;

				mainDescriptorCount++;
			}
		}

		for (const auto& [set, bindings] : resources.storageBuffers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& info = m_allocatedDescriptorPointers[set][binding];

				if (data.isWrite)
				{
					info.viewType = D3D12ViewType::UAV;
				}
				else
				{
					info.viewType = D3D12ViewType::SRV;
				}

				mainDescriptorCount++;
			}
		}

		for (const auto& [set, bindings] : resources.storageImages)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& info = m_allocatedDescriptorPointers[set][binding];
				info.viewType = D3D12ViewType::UAV;
				mainDescriptorCount++;
			}
		}

		for (const auto& [set, bindings] : resources.images)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& info = m_allocatedDescriptorPointers[set][binding];
				info.viewType = D3D12ViewType::SRV;
				mainDescriptorCount++;
			}
		}

		for (const auto& [set, bindings] : resources.samplers)
		{
			for (const auto& [binding, data] : bindings)
			{
				auto& info = m_allocatedDescriptorPointers[set][binding];
				info.viewType = D3D12ViewType::Sampler;
				samplerDescriptorCount++;
			}
		}

		CreateDescriptorHeaps(mainDescriptorCount, samplerDescriptorCount);
		AllocateDescriptors();
	}

	void D3D12DescriptorTable::Release()
	{
		m_allocatedDescriptorPointers.clear();

		m_mainHeap = nullptr;
		m_samplerHeap = nullptr;
	}

	void D3D12DescriptorTable::CreateDescriptorHeaps(uint32_t mainDescriptorCount, uint32_t samplerDescriptorCount)
	{
		if (mainDescriptorCount > 0)
		{
			DescriptorHeapSpecification specification{};
			specification.descriptorType = D3D12DescriptorType::CBV_SRV_UAV;
			specification.maxDescriptorCount = mainDescriptorCount;
			specification.supportsGPUDescriptors = true;

			m_mainHeap = CreateScope<D3D12DescriptorHeap>(specification);
		}

		if (samplerDescriptorCount > 0)
		{
			DescriptorHeapSpecification specification{};
			specification.descriptorType = D3D12DescriptorType::Sampler;
			specification.maxDescriptorCount = samplerDescriptorCount;
			specification.supportsGPUDescriptors = true;

			m_samplerHeap = CreateScope<D3D12DescriptorHeap>(specification);
		}
	}

	void D3D12DescriptorTable::AllocateDescriptors()
	{
		for (auto& [set, bindings] : m_allocatedDescriptorPointers)
		{
			for (auto& [binding, data] : bindings)
			{
				if (data.viewType == D3D12ViewType::CBV ||
					data.viewType == D3D12ViewType::SRV ||
					data.viewType == D3D12ViewType::UAV)
				{
					data.pointer = m_mainHeap->Allocate();
				}
				else if (data.viewType == D3D12ViewType::Sampler)
				{
					data.pointer = m_samplerHeap->Allocate();
				}
			}
		}
	}
}
