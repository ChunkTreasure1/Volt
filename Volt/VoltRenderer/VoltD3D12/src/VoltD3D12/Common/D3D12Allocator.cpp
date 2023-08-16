#include "dxpch.h"
#include "D3D12Allocator.h"

#include "VoltRHI/Graphics/GraphicsDevice.h"

#include "VoltD3D12/Graphics/D3D12PhysicalGraphicsDevice.h"

namespace Volt::RHI
{
	void D3d12Allocator::Initialize()
	{
		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		auto adapter = GraphicsContext::GetPhysicalDevice()->GetHandle<IDXGIAdapter1*>();

		D3D12MA::ALLOCATOR_DESC allocDesc = {};
		allocDesc.pAdapter = adapter;
		allocDesc.pDevice = device;
		allocDesc.Flags = D3D12MA::ALLOCATOR_FLAG_ALWAYS_COMMITTED;
		allocDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;

		VT_D3D12_CHECK(D3D12MA::CreateAllocator(&allocDesc, &s_Allcator));
		s_ID = 5;
	}
	void D3d12Allocator::Allocate(AllocatedImage& image, D3D12_RESOURCE_DESC& resourceDesc, D3D12MA::ALLOCATION_DESC& allocationDesc, D3D12_RESOURCE_STATES intialState)
	{
		VT_D3D12_CHECK(
			s_Allcator->CreateResource(
				&allocationDesc,
				&resourceDesc,
				intialState,
				nullptr,
				&image.allocation,
				IID_PPV_ARGS(&image.texture)));

		image.id = s_ID++;

		image.sizeOfBuffer = image.allocation->GetSize();

		//HY_ALLOC_PRINT("DxAllocator: id {0} Allocating image: {1} bytes", image.id, image.sizeOfBuffer);

		s_AllocateDestructorOrder.push_back(image.id);
		s_DestroyFunctions[image.id] = [&, image]()
		{
			image.texture->Release();
			image.allocation->Release();
			//HY_ALLOC_PRINT("DxAllocator: id {0} Deallocating image: {1} bytes", image.id, image.sizeOfBuffer);
		};
	}
	void D3d12Allocator::Allocate(AllocatedBuffer& image, D3D12_RESOURCE_DESC& resourceDesc, D3D12MA::ALLOCATION_DESC& allocationDesc, D3D12_RESOURCE_STATES intialState)
	{
		VT_D3D12_CHECK(
			s_Allcator->CreateResource(
				&allocationDesc,
				&resourceDesc,
				intialState,
				nullptr,
				&image.allocation,
				IID_PPV_ARGS(&image.buffer)));
		image.id = s_ID++;
		image.sizeOfBuffer = image.allocation->GetSize();
		s_AllocateDestructorOrder.push_back(image.id);
		s_DestroyFunctions[image.id] = [&, image]()
		{
			image.buffer->Release();
			image.allocation->Release();
		};
	}
	void D3d12Allocator::DeAllocate(AllocatedImage& image)
	{
		image.texture->Release();
		image.allocation->Release();
		s_DestroyFunctions.erase(image.id);
		auto it = std::find(s_AllocateDestructorOrder.begin(), s_AllocateDestructorOrder.end(), image.id);
		if (it != s_AllocateDestructorOrder.end())
		{
			s_AllocateDestructorOrder.erase(it);
		}
	}
	void D3d12Allocator::DeAllocate(AllocatedBuffer& buffer)
	{
		buffer.buffer->Release();
		buffer.allocation->Release();
		s_DestroyFunctions.erase(buffer.id);
		auto it = std::find(s_AllocateDestructorOrder.begin(), s_AllocateDestructorOrder.end(), buffer.id);
		if (it != s_AllocateDestructorOrder.end())
		{
			s_AllocateDestructorOrder.erase(it);
		}
	}
	void D3d12Allocator::Flush()
	{
		for (int32_t Index = static_cast<int32_t>(s_AllocateDestructorOrder.size() - 1u); Index >= 0; Index--)
		{
			s_DestroyFunctions[s_AllocateDestructorOrder[Index]]();
		}

		for (auto& it : s_DestructionQueue)
		{
			it();
		}
		s_DestructionQueue.clear();
	}
	void D3d12Allocator::Shutdown()
	{
		VT_D3D12_DELETE(s_Allcator);
	}
}
