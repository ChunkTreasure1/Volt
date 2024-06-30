#include "rhipch.h"
#include "DescriptorTable.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<DescriptorTable> DescriptorTable::Create(const DescriptorTableCreateInfo& specification)
	{
		return RHIProxy::GetInstance().CreateDescriptorTable(specification);
	}
}
