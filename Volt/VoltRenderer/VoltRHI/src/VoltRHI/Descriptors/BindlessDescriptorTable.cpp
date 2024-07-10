#include "rhipch.h"
#include "BindlessDescriptorTable.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<BindlessDescriptorTable> BindlessDescriptorTable::Create()
	{
		return RHIProxy::GetInstance().CreateBindlessDescriptorTable();
	}
}
