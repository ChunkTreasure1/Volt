#pragma once
#include "VoltD3D12/Common/D3D12AllocatorForward.h"

#include <dxgi1_6.h>
#include <deque>
#include <functional>
#include <unordered_map>
#include <dxpch.h>

#include "VoltD3D12/Common/D3D12MemAlloc.h"



namespace Volt::RHI
{
	

	class D3d12Allocator
	{
	public:
		static void Initialize();
		static void Allocate(AllocatedImage& image, D3D12_RESOURCE_DESC& resourceDesc, D3D12MA::ALLOCATION_DESC& allocationDesc, D3D12_RESOURCE_STATES intialState);
		static void Allocate(AllocatedBuffer& image, D3D12_RESOURCE_DESC& resourceDesc, D3D12MA::ALLOCATION_DESC& allocationDesc, D3D12_RESOURCE_STATES intialState);
		static void DeAllocate(AllocatedImage& image);
		static void DeAllocate(AllocatedBuffer& buffer);

		//static void MapMemory(AllocatedBuffer& buffer, RHIRange* range, void*& mappedMemory);
		//static void UnMapMemory(AllocatedBuffer& buffer);

		//static D3D12_HEAP_TYPE GetHeapTypeFromResourceState(RHIResourceState state);

		static void Flush();
		static void Shutdown();
	private:
		inline static D3D12MA::Allocator* s_Allcator;
		inline static D3D12MA::Pool* s_Pool;
		inline static uint32_t s_ID;
		inline static std::unordered_map<uint32_t, std::function<void()>> s_DestroyFunctions;
		inline static std::vector<uint32_t> s_AllocateDestructorOrder;
		inline static std::deque<std::function<void()>> s_DestructionQueue;
	};
}
