#pragma once

#include "D3D12RHIModule/Descriptors/DescriptorCommon.h"

namespace Volt::RHI::DescriptorUtility
{
	extern D3D12DescriptorPointer AllocateDescriptorPointer(D3D12DescriptorType type);
	extern void FreeDescriptorPointer(D3D12DescriptorPointer descriptorPointer);
}
