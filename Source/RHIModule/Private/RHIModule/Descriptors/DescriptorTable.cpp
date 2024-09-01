#include "rhipch.h"

#include "RHIModule/Descriptors/DescriptorTable.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<DescriptorTable> DescriptorTable::Create(const DescriptorTableCreateInfo& specification)
	{
		return RHIProxy::GetInstance().CreateDescriptorTable(specification);
	}
}
