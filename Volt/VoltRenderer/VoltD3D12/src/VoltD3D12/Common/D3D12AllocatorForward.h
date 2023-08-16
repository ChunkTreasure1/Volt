#pragma once
#include <cstdint>

struct ID3D12Resource;
namespace D3D12MA
{
	struct Allocation;
	struct ALLOCATION_DESC;
}

namespace Volt::RHI
{
	struct AllocatedBuffer
	{
		uint32_t id;
		ID3D12Resource* buffer;
		D3D12MA::Allocation* allocation;
		size_t sizeOfBuffer;
	};

	struct AllocatedImage
	{
		uint32_t id;
		ID3D12Resource* texture;
		D3D12MA::Allocation* allocation;
		size_t sizeOfBuffer;
	};
}
