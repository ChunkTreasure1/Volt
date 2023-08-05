#pragma once
#include "VoltRHI/Core/Core.h"

struct ID3D12DescriptorHeap;

namespace Volt::RHI
{
	class D3D12DescriptorHeapManager
	{
	public:

		static void CreateNewRTVHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);
		static void CreateNewDSVHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);

		FORCEINLINE [[nodiscard]] static uint32_t GetRTVSize() { return s_Instance->m_RTVDescriptorSize; }
		FORCEINLINE [[nodiscard]] static ID3D12DescriptorHeap* GetRTVHeap() { return s_Instance->m_RTVDescriptorHeap; }

	private:
		D3D12DescriptorHeapManager();

		static void Validate();

		inline static Scope<D3D12DescriptorHeapManager> s_Instance;

		ID3D12DescriptorHeap* m_RTVDescriptorHeap;
		uint32_t m_RTVDescriptorSize;
		uint32_t m_CurrentRTVOffset;

		ID3D12DescriptorHeap* m_DSVDescriptorHeap;
		uint32_t m_DSVDescriptorSize;
		uint32_t m_CurrentDSVOffset;
	};
}
