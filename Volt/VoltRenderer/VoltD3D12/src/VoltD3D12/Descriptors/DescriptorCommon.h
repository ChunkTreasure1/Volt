#pragma once

#include "VoltD3D12/Common/D3D12Common.h"

#include <CoreUtilities/Core.h>

#include <cstdint>
#include <limits>

namespace Volt::RHI
{
	struct D3D12DescriptorPointer
	{
		uint64_t cpuPointer = std::numeric_limits<uint64_t>::max();
		uint64_t gpuPointer = std::numeric_limits<uint64_t>::max();
		size_t parentHeapHash = 0;

		VT_NODISCARD VT_INLINE bool IsValid() const { return cpuPointer != std::numeric_limits<uint64_t>::max() && parentHeapHash != 0; }
		VT_NODISCARD VT_INLINE uint64_t GetCPUPointer() const { return cpuPointer; }
		VT_NODISCARD VT_INLINE uint64_t GetGPUPointer() const { return gpuPointer; }
	};

	enum class D3D12DescriptorType
	{
		None,
		RTV,
		DSV,
		CBV_SRV_UAV,
		Sampler
	};

	struct DescriptorCopyInfo
	{
		D3D12DescriptorPointer srcPointer;
		D3D12DescriptorPointer dstPointer;
	};

	struct AllocatedDescriptorInfo
	{
		D3D12DescriptorPointer pointer;
		D3D12ViewType viewType;
		uint32_t descriptorIndex;
	};
}
