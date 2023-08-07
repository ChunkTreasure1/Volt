#include "dxpch.h"
#include "D3D12DescriptorHeapManager.h"
#include <VoltD3D12/Graphics/D3D12GraphicsDevice.h>

namespace Volt::RHI
{
	constexpr uint32_t MaxAmountOfRTVs = 1000;
	constexpr uint32_t MaxAmountOfDSVs = 1000;

	D3D12DescriptorHeapManager::D3D12DescriptorHeapManager()
	{
		auto d3d12Device = GraphicsContext::GetDevice()->As<D3D12GraphicsDevice>();
		auto device = d3d12Device->GetHandle<ID3D12Device2*>();

		m_CurrentRTVOffset = 0;

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NumDescriptors = MaxAmountOfRTVs;
		rtvHeapDesc.NodeMask = 0;

		device->CreateDescriptorHeap(&rtvHeapDesc, VT_D3D12_ID(m_RTVDescriptorHeap));

		m_RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		m_CurrentDSVOffset = 0;

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NumDescriptors = MaxAmountOfDSVs;
		dsvHeapDesc.NodeMask = 0;

		device->CreateDescriptorHeap(&dsvHeapDesc, VT_D3D12_ID(m_DSVDescriptorHeap));
		m_DSVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	uint32_t D3D12DescriptorHeapManager::CreateNewRTVHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
	{
		Validate();
		handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(s_Instance->m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), s_Instance->m_CurrentRTVOffset, s_Instance->m_RTVDescriptorSize);
		auto currentOffset = s_Instance->m_CurrentRTVOffset;
		s_Instance->m_CurrentRTVOffset++;
		return currentOffset;
	}

	void D3D12DescriptorHeapManager::CreateRTVHandleFromID(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, const uint32_t id)
	{
		Validate();

		handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(s_Instance->m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), id, s_Instance->m_RTVDescriptorSize);
	}

	uint32_t D3D12DescriptorHeapManager::CreateNewDSVHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
	{
		Validate();

		handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(s_Instance->m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), s_Instance->m_CurrentDSVOffset, s_Instance->m_DSVDescriptorSize);
		auto currentOffset = s_Instance->m_CurrentDSVOffset;
		s_Instance->m_CurrentDSVOffset++;
		return currentOffset;
	}

	void D3D12DescriptorHeapManager::Validate()
	{
		if (s_Instance)
		{
			return;
		}

		s_Instance = std::make_unique<D3D12DescriptorHeapManager>();
	}
}
