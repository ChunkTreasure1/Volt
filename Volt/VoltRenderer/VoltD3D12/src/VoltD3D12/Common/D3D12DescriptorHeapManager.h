#pragma once
#include "VoltRHI/Core/Core.h"

struct ID3D12DescriptorHeap;

namespace Volt::RHI
{
	class D3D12DescriptorHeapManager
	{
	public:
		D3D12DescriptorHeapManager();

		static uint32_t CreateNewRTVHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);

		//REMEMBER, THIS ID NEEDS TO BE VALID ID FROM THIS CLASS INORDER TO WORK! IF ID IS CREATED EXTERNALLY FROM THIS CLASS THERE WILL BE UNEXPECTED CRASHES!!!
		static void CreateRTVHandleFromID(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, const uint32_t id);

		static uint32_t CreateNewDSVHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);

		FORCEINLINE [[nodiscard]] static uint32_t GetRTVSize() { return s_Instance->m_RTVDescriptorSize; }
		FORCEINLINE [[nodiscard]] static ID3D12DescriptorHeap* GetRTVHeap() { return s_Instance->m_RTVDescriptorHeap; }

	private:

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
