#include "rhipch.h"
#include "BindlessDescriptorTable.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<BindlessDescriptorTable> BindlessDescriptorTable::Create(const uint64_t framesInFlight)
	{
		return RHIProxy::GetInstance().CreateBindlessDescriptorTable(framesInFlight);
	}
}
